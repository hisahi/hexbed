/****************************************************************************/
/*                                                                          */
/* HexBed -- Hex editor                                                     */
/* Copyright (c) 2021-2022 Sampo Hippeläinen (hisahi)                       */
/*                                                                          */
/* This program is free software: you can redistribute it and/or modify     */
/* it under the terms of the GNU General Public License as published by     */
/* the Free Software Foundation, either version 3 of the License, or        */
/* (at your option) any later version.                                      */
/*                                                                          */
/* This program is distributed in the hope that it will be useful,          */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of           */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the            */
/* GNU General Public License for more details.                             */
/*                                                                          */
/* You should have received a copy of the GNU General Public License        */
/* along with this program.  If not, see <https://www.gnu.org/licenses/>.   */
/*                                                                          */
/****************************************************************************/
// file/bmmap.cc -- impl for the mmap document buffer

#define _POSIX_C_SOURCE 200809L

#include "file/bmmap.hh"

#include <cstring>

#include "common/error.hh"
#include "common/logger.hh"
#include "common/memory.hh"
#include "file/bfile.hh"

namespace hexbed {

#if HEXBED_MMAP_POSIX
#define MMAP_MEM_NULL (reinterpret_cast<byte*>(MAP_FAILED))
#else
#define MMAP_MEM_NULL nullptr
#endif

class HexBedBufferMmapVbuf : public VirtualBuffer {
  public:
    HexBedBufferMmapVbuf(HexBedBufferMmap* mmap, std::FILE* wf)
        : mmap_(mmap), wf_(wf) {}
    void raw(bufsize n, const byte* r) {
        errno = 0;
        if (!std::fwrite(r, n, 1, wf_)) throw errno_to_exception(errno);
    }
    void copy(bufsize n, bufsize o) {
        bufsize x;
        byte* p;
        while (n && (p = mmap_->extent(o, x))) {
            errno = 0;
            if (x > n) x = n;
            if (!std::fwrite(p, x, 1, wf_)) throw errno_to_exception(errno);
            o += x;
            n -= x;
        }
    }

  private:
    HexBedBufferMmap* mmap_;
    std::FILE* wf_;
};

HexBedBufferMmap::HexBedBufferMmap(const std::string& filename)
    : fd_(filename), page_(0) {
#if HEXBED_MMAP_POSIX
    struct stat sb;
    if (fstat(fd_.fd, &sb) == -1) throw errno_to_exception(errno);
    sz_ = static_cast<bufsize>(sb.st_size);
#else
    sz_ = 0;
#endif
#if HEXBED_MMAP_POSIX
    unsigned long syspagesize =
        static_cast<unsigned long>(sysconf(_SC_PAGE_SIZE));
    if (std::popcount(syspagesize) != 1)
        throw std::runtime_error("system page size not a power of two...?");
    pagesz_ = std::max<bufsize>(std::bit_width<bufsize>(syspagesize), 16);
    int flags = MAP_PRIVATE;
#if HEXBED_MMAP_LINUX
    flags |= MAP_NORESERVE;
#endif
    mem_ = reinterpret_cast<byte*>(
        mmap(nullptr, 1 << pagesz_, PROT_READ, flags, fd_.fd, 0));
    if (mem_ == MAP_FAILED) throw errno_to_exception(errno);
#endif
}

HexBedBufferMmap::HexBedBufferMmap(HexBedBufferMmap&& move)
    : fd_(std::move(move.fd_)),
      mem_(std::exchange(move.mem_, MMAP_MEM_NULL)),
      sz_(move.sz_),
      page_(move.page_),
      pagesz_(move.pagesz_) {}

HexBedBufferMmap& HexBedBufferMmap::operator=(HexBedBufferMmap&& move) {
    fd_ = std::move(move.fd_);
    mem_ = std::exchange(move.mem_, MMAP_MEM_NULL);
    sz_ = move.sz_;
    page_ = move.page_;
    pagesz_ = move.pagesz_;
    return *this;
}

void HexBedBufferMmap::drop() noexcept {
#if HEXBED_MMAP_POSIX
    if (mem_ != MAP_FAILED)
        munmap(std::exchange(mem_, MMAP_MEM_NULL), 1 << pagesz_);
#endif
    fd_.drop();
}

HexBedBufferMmap::~HexBedBufferMmap() noexcept { drop(); }

byte* HexBedBufferMmap::extent(bufsize offset, bufsize& size) {
    if (offset >= sz_) {
        size = 0;
        return nullptr;
    }
    bufsize targetpage = offset >> pagesz_;
    bufsize targetbase = targetpage << pagesz_;
    bufsize pagesize = 1 << pagesz_;
    bufsize targetoff = offset & (pagesize - 1);
    if (page_ != targetpage) {
        // mremap
#if HEXBED_MMAP_POSIX
        byte* newmap =
            reinterpret_cast<byte*>(mmap(mem_, pagesize, PROT_READ, MAP_PRIVATE,
                                         fd_.fd, targetpage << pagesz_));
        if (newmap == MAP_FAILED) throw errno_to_exception(errno);
        if (mem_ != newmap) munmap(mem_, pagesize);
        mem_ = newmap;
#else
        throw std::runtime_error("remapping not supported");
#endif
    }
    size = std::min(sz_ - targetbase, pagesize) - targetoff;
    return mem_ + targetoff;
}

bufsize HexBedBufferMmap::read(bufoffset offset, bytespan data) {
    if (!fd_) throw system_io_error("file is closed");
    // TODO: we must handle SIGBUS and SIGSEGV in case there is a read error
    bufsize n = data.size(), x;
    byte* di = data.data();
    byte* si;
    while (n && (si = extent(offset, x))) {
        if (x > n) x = n;
        memCopy(di, si, x);
        offset += x;
        di += x;
        n -= x;
    }
    return data.size() - n;
}

void HexBedBufferMmap::write(WriteCallback write, const std::string& filename) {
    std::string tmpfn;
    FILE_unique_ptr fp = fopen_replace_before(filename, tmpfn);

    HexBedBufferMmapVbuf vbuf(this, fp.get());
    write(vbuf);

    errno = 0;
    if (std::fflush(fp.get())) throw errno_to_exception(errno);

    // close the original file
    drop();

    // and replace it
    fopen_replace_after(filename, tmpfn, std::move(fp));
}

void HexBedBufferMmap::writeOverlay(WriteCallback write,
                                    const std::string& filename) {
    HexBedBufferMmap::write(write, filename);
}

void HexBedBufferMmap::writeNew(WriteCallback write,
                                const std::string& filename) {
    if (!fd_) throw system_io_error("file is closed");
    errno = 0;
    auto fp = fopen_unique(filename.c_str(), "wb");
    if (!fp) throw errno_to_exception(errno);
    std::setvbuf(fp.get(), NULL, _IONBF, 0);

    HexBedBufferMmapVbuf vbuf(this, fp.get());
    write(vbuf);

    errno = 0;
    if (std::fflush(fp.get())) throw errno_to_exception(errno);
}

void HexBedBufferMmap::writeCopy(WriteCallback write,
                                 const std::string& filename) {
    writeNew(write, filename);
}

bufsize HexBedBufferMmap::size() const noexcept { return sz_; }

};  // namespace hexbed
