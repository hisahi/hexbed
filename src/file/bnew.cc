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
// file/bnew.cc -- impl for the new document buffer

#include "file/bnew.hh"

#include "common/error.hh"
#include "common/logger.hh"
#include "file/bfile.hh"

namespace hexbed {

class HexBedBufferNewVbuf : public VirtualBuffer {
  public:
    HexBedBufferNewVbuf(std::FILE* f) : f_(f) {}
    void raw(bufsize n, const byte* r) {
        errno = 0;
        if (!std::fwrite(r, n, 1, f_)) throw errno_to_exception(errno);
    }
    void copy(bufsize n, bufsize o) {}

  private:
    std::FILE* f_;
};

bufsize HexBedBufferNew::read(bufoffset offset, bytespan data) { return 0; }

void HexBedBufferNew::write(HexBedContext& ctx, WriteCallback write,
                            const std::string& filename) {
    HEXBED_ASSERT(0, "cannot write() a new file");
}

void HexBedBufferNew::writeOverlay(HexBedContext& ctx, WriteCallback write,
                                   const std::string& filename) {
    HEXBED_ASSERT(0, "cannot write() a new file");
}

void HexBedBufferNew::writeNew(HexBedContext& ctx, WriteCallback write,
                               const std::string& filename) {
    if (ctx.shouldBackup()) makeBackupOf(ctx, filename);
    errno = 0;
    auto fp = fopen_unique(filename.c_str(), "wb");
    if (!fp) throw errno_to_exception(errno);

    std::setvbuf(fp.get(), NULL, _IONBF, 0);
    HexBedBufferNewVbuf vbuf(fp.get());
    write(vbuf);

    errno = 0;
    if (std::fflush(fp.get())) throw errno_to_exception(errno);
}

void HexBedBufferNew::writeCopy(HexBedContext& ctx, WriteCallback write,
                                const std::string& filename) {
    writeNew(ctx, write, filename);
}

bufsize HexBedBufferNew::size() const noexcept { return 0; }

};  // namespace hexbed
