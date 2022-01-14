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
// file/bmmap.hh -- header for the mmap document buffer

#ifndef HEXBED_FILE_BMMAP_HH
#define HEXBED_FILE_BMMAP_HH

#define _POSIX_C_SOURCE 200809L

#include <memory>

#include "common/error.hh"
#include "file/context.hh"
#include "file/document.hh"

#define HEXBED_MMAP_OK 1
#ifdef __linux__
#define HEXBED_MMAP_LINUX 1
#define HEXBED_MMAP_POSIX 1
#include <sys/mman.h>
#else
#undef HEXBED_MMAP_OK
#endif

#ifdef NDEBUG
// mmap I/O errors will currently cause a crash (because SIGBUS/SIGSEGV is
// not handled) so don't use it on release builds
#undef HEXBED_MMAP_OK
#endif

#if HEXBED_MMAP_POSIX
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#endif

namespace hexbed {

#if HEXBED_MMAP_POSIX
struct file_wrapper {
    int fd;
    inline file_wrapper(const std::filesystem::path& filename,
                        int flags = O_RDONLY) {
        fd = open(filename.c_str(), flags);
        if (fd == -1) throw errno_to_exception(errno);
    }
    inline file_wrapper(int fd) : fd(fd) {}

    inline file_wrapper(const file_wrapper& copy) = delete;
    inline file_wrapper& operator=(const file_wrapper& copy) = delete;

    inline file_wrapper(file_wrapper&& move)
        : file_wrapper(std::exchange(move.fd, -1)) {}

    inline file_wrapper& operator=(file_wrapper&& move) {
        fd = std::exchange(move.fd, -1);
        return *this;
    }
    inline operator bool() const noexcept { return fd != -1; }

    inline void drop() noexcept {
        if (fd != -1) close(std::exchange(fd, -1));
    }

    inline ~file_wrapper() noexcept {
        if (fd != -1) close(fd);
    }
};
#else
struct file_wrapper {
    inline file_wrapper() { throw std::runtime_error("not supported"); }
    inline file_wrapper(const std::filesystem::path& filename, int flags = 0)
        : file_wrapper() {}

    inline file_wrapper(const file_wrapper& copy) = delete;
    inline file_wrapper& operator=(const file_wrapper& copy) = delete;

    inline file_wrapper(file_wrapper&& move) : file_wrapper() {}
    inline file_wrapper& operator=(file_wrapper&& move) { return *this; }
    inline operator bool() const noexcept { return false; }

    inline void drop() noexcept {}

    inline ~file_wrapper() noexcept {}
};
#endif

class HexBedBufferMmap : public HexBedBuffer {
  public:
    HexBedBufferMmap(const std::filesystem::path& filename);
    bufsize read(bufoffset offset, bytespan data);
    void write(HexBedContext& ctx, WriteCallback write,
               const std::filesystem::path& filename);
    void writeOverlay(HexBedContext& ctx, WriteCallback write,
                      const std::filesystem::path& filename);
    void writeNew(HexBedContext& ctx, WriteCallback write,
                  const std::filesystem::path& filename);
    void writeCopy(HexBedContext& ctx, WriteCallback write,
                   const std::filesystem::path& filename);
    bufsize size() const noexcept;

    byte* extent(bufsize offset, bufsize& size);

    HexBedBufferMmap(const HexBedBufferMmap& copy) = delete;
    HexBedBufferMmap& operator=(const HexBedBufferMmap& copy) = delete;

    HexBedBufferMmap(HexBedBufferMmap&& move);
    HexBedBufferMmap& operator=(HexBedBufferMmap&& move);

    ~HexBedBufferMmap() noexcept;

  private:
    file_wrapper fd_;
    byte* mem_;
    bufsize sz_;
    bufsize page_;
    bufsize pagesz_;

    void drop() noexcept;
};

};  // namespace hexbed

#endif /* HEXBED_FILE_BMMAP_HH */
