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
// file/bfile.cc -- impl for the FILE document buffer

#define _POSIX_C_SOURCE 200809L

#include "file/bfile.hh"

#include <cstdio>
#include <filesystem>
#include <memory>
#include <random>

#include "common/error.hh"
#include "common/logger.hh"

#if defined(_POSIX_VERSION)
#include <sys/types.h>
#include <unistd.h>
#endif

namespace hexbed {

static std::random_device rd;
static std::default_random_engine re{rd()};

constexpr long long_max = std::numeric_limits<long>::max();
constexpr unsigned long ulong_max = std::numeric_limits<unsigned long>::max();

template <typename T>
static int fseekto_massive_(std::FILE* f, bufsize o) {
    static_assert(
        ((static_cast<unsigned long>(long_max) << 1UL) | 1UL) == ulong_max,
        "not 2's complement?");
    if constexpr (sizeof(bufsize) <= sizeof(unsigned long) &&
                  static_cast<bufsize>(BUFSIZE_MAX) <=
                      static_cast<bufsize>(ulong_max)) {
        if (o < long_max) return fseek(f, static_cast<long>(o), SEEK_SET);
        if (fseek(f, static_cast<long>(long_max), SEEK_SET)) return -1;
        o -= long_max;
        if (o >= long_max) {
            if (fseek(f, static_cast<long>(long_max), SEEK_CUR)) return -1;
            o -= long_max;
        }
        return fseek(f, static_cast<long>(o), SEEK_CUR);
    } else {
#if defined(_MSC_VER)
        constexpr __int64 int64_max = 0x7FFFFFFFFFFFFFFFULL;
        if (o < int64_max)
            return _fseeki64(f, static_cast<__int64>(o), SEEK_SET);
        if (_fseeki64(f, static_cast<__int64>(int64_max), SEEK_SET)) return -1;
        o -= int64_max;
        if (o >= int64_max) {
            if (_fseeki64(f, static_cast<__int64>(int64_max), SEEK_CUR))
                return -1;
            o -= int64_max;
        }
        return _fseeki64(f, static_cast<__int64>(o), SEEK_CUR);
#else
        // if you change bufsize to long which is 32 bits wide, be aware that
        // doing so restricts HexBed to work only with files if the size fits
        // in 32 bits
        static_assert(std::is_same_v<T, float>,
                      "bufsize is larger than long! cannot handle large files");
        return -1;
#endif
    }
}

template <typename T>
static bufsize ftell_massive_(std::FILE* f) {
    if constexpr (sizeof(bufsize) <= sizeof(unsigned long) &&
                  static_cast<bufsize>(BUFSIZE_MAX) <=
                      static_cast<bufsize>(ulong_max)) {
        return static_cast<bufsize>(std::ftell(f));
    } else {
#if defined(_MSC_VER)
        return static_cast<bufsize>(_ftelli64(f));
#else
        // if you change bufsize to long which is 32 bits wide, be aware that
        // doing so restricts HexBed to work only with files if the size fits
        // in 32 bits
        static_assert(std::is_same_v<T, float>,
                      "bufsize is larger than long! cannot handle large files");
        return 0;
#endif
    }
}

static int fseekto_massive(std::FILE* f, bufsize o) {
    return fseekto_massive_<int>(f, o);
}

static bufsize ftell_massive(std::FILE* f) { return ftell_massive_<int>(f); }

static bufsize fcopy(std::FILE* wf, std::FILE* rf, bufsize n) {
    byte buf[BUFSIZ];
    bufsize o = 0;
    while (n) {
        size_t c = std::min<bufsize>(sizeof(buf), n);
        size_t q = std::fread(buf, 1, c, rf);
        if (!std::fwrite(buf, q, 1, wf)) break;
        n -= q;
        o += q;
        if (c < q) break;
    }
    return o;
}

// static int ftruncate(std::FILE* f);

#if defined(_POSIX_VERSION)
// truncate at file pointer
static int ftruncate(std::FILE* f) {
    off_t pos = ftell_massive(f);
    if (pos < 0) return -1;
    return ::ftruncate(fileno(f), pos);
}
#define HAVE_TRUNCATE 1
#else
#define HAVE_TRUNCATE 0
#endif

class HexBedBufferFileVbuf : public VirtualBuffer {
  public:
    HexBedBufferFileVbuf(std::FILE* rf, std::FILE* wf) : rf_(rf), wf_(wf) {}
    void raw(bufsize n, const byte* r) {
        errno = 0;
        if (!std::fwrite(r, n, 1, wf_)) throw errno_to_exception(errno);
    }
    void copy(bufsize n, bufsize o) {
        errno = 0;
        if (fseekto_massive(rf_, o)) throw errno_to_exception(errno);
        bufsize q = fcopy(wf_, rf_, n);
        if (q < n && (std::ferror(rf_) || std::ferror(wf_)))
            throw errno_to_exception(errno);
    }

  private:
    std::FILE* rf_;
    std::FILE* wf_;
};

class HexBedBufferFileVbufOverlay : public VirtualBuffer {
  public:
    HexBedBufferFileVbufOverlay(std::FILE* f) : f_(f) {}
    void raw(bufsize n, const byte* r) {
        errno = 0;
        if (!std::fwrite(r, n, 1, f_)) throw errno_to_exception(errno);
#ifndef NDEBUG
        total_ += n;
#endif
    }
    void copy(bufsize n, bufsize o) {
        errno = 0;
#ifndef NDEBUG
        HEXBED_ASSERT(total_ == o);
        total_ = o + n;
#endif
        if (fseekto_massive(f_, o + n)) throw errno_to_exception(errno);
    }

  private:
    std::FILE* f_;
#ifndef NDEBUG
    bufsize total_{0};
#endif
};

HexBedBufferFile::HexBedBufferFile(const std::string& filename)
    : f_((errno = 0, fopen_unique(filename.c_str(), "rb"))) {
    if (!f_) throw errno_to_exception(errno);
    updateSize();
}

void HexBedBufferFile::updateSize() {
    errno = 0;
    if (std::fseek(f_.get(), 0, SEEK_END)) throw errno_to_exception(errno);
    bufsize sz = ftell_massive(f_.get());
    if (sz != BUFSIZE_MAX) sz_ = sz;
}

bufsize HexBedBufferFile::read(bufoffset offset, bytespan data) {
    errno = 0;
    if (!f_) throw system_io_error("file is closed");
    if (fseekto_massive(f_.get(), offset)) throw errno_to_exception(errno);
    bufsize r = std::fread(data.data(), 1, data.size(), f_.get());
    if (r < data.size() && std::ferror(f_.get()))
        throw errno_to_exception(errno);
    return r;
}

FILE_unique_ptr fopen_replace_before(const std::string& filename,
                                     std::string& tempfilename) {
    std::string tmpfn = filename + ".hexbedtmp000000";
    size_t rndoff = tmpfn.size() - 6;
    FILE_unique_ptr fp;
    std::uniform_int_distribution distr{0, 15};
    for (int tries = 0; tries < 20; ++tries) {
        errno = 0;
        char* buf = tmpfn.data() + rndoff;
        buf[0] = HEX_LOWERCASE[tries & 15];
        for (int i = 1; i < 6; ++i) buf[i] = HEX_LOWERCASE[distr(re)];
        fp = fopen_unique(tmpfn.c_str(), "wxb");
        if (fp) break;
    }
    if (!fp) throw errno_to_exception(errno);
    std::setvbuf(fp.get(), NULL, _IONBF, 0);
    tempfilename = tmpfn;
    return fp;
}

void fopen_replace_after(const std::string& filename,
                         const std::string& tempfilename, FILE_unique_ptr&& f) {
    f = nullptr;
    std::filesystem::rename(tempfilename, filename);
}

void HexBedBufferFile::write(WriteCallback write, const std::string& filename) {
    if (!f_) throw system_io_error("file is closed");
    std::string tmpfn;
    FILE_unique_ptr fp = fopen_replace_before(filename, tmpfn);

    HexBedBufferFileVbuf vbuf(f_.get(), fp.get());
    write(vbuf);

    errno = 0;
    if (std::fflush(fp.get())) throw errno_to_exception(errno);

    // close the original file
    f_ = nullptr;

    // and replace it
    fopen_replace_after(filename, tmpfn, std::move(fp));
}

void HexBedBufferFile::writeOverlay(WriteCallback write,
                                    const std::string& filename) {
#if HAVE_TRUNCATE
    if (!f_) throw system_io_error("file is closed");
    auto fp = freopen_unique(nullptr, "r+b", f_);
    if (!fp) {
        int old_errno = errno;
        f_ = fopen_unique(filename.c_str(), "rb");
        throw errno_to_exception(old_errno);
    }

    std::setvbuf(fp.get(), NULL, _IONBF, 0);
    HexBedBufferFileVbufOverlay vbuf(fp.get());
    write(vbuf);

    errno = 0;
    if (ftruncate(fp.get())) throw errno_to_exception(errno);
    errno = 0;
    if (std::fflush(fp.get())) throw errno_to_exception(errno);
#else
    HexBedBufferFile::write(write, filename);
#endif
}

void HexBedBufferFile::writeNew(WriteCallback write,
                                const std::string& filename) {
    if (!f_) throw system_io_error("file is closed");
    errno = 0;
    auto fp = fopen_unique(filename.c_str(), "wb");
    if (!fp) throw errno_to_exception(errno);

    std::setvbuf(fp.get(), NULL, _IONBF, 0);
    HexBedBufferFileVbuf vbuf(f_.get(), fp.get());
    write(vbuf);

    errno = 0;
    if (std::fflush(fp.get())) throw errno_to_exception(errno);
}

void HexBedBufferFile::writeCopy(WriteCallback write,
                                 const std::string& filename) {
    writeNew(write, filename);
}

bufsize HexBedBufferFile::size() const noexcept { return sz_; }

};  // namespace hexbed
