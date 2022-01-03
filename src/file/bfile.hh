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
// file/bfile.hh -- header for the FILE document buffer

#ifndef HEXBED_FILE_BFILE_HH
#define HEXBED_FILE_BFILE_HH

#include <cstdio>
#include <memory>

#include "file/document.hh"

namespace hexbed {

using FILE_unique_ptr =
    std::unique_ptr<std::FILE, std::function<int(std::FILE*)>>;

inline FILE_unique_ptr fopen_unique(const char* filename, const char* mode) {
    return FILE_unique_ptr(std::fopen(filename, mode), [](std::FILE* fp) {
        return fp ? std::fclose(fp) : 0;
    });
}

inline FILE_unique_ptr freopen_unique(const char* filename, const char* mode,
                                      FILE_unique_ptr& fp) {
    return FILE_unique_ptr(
        std::freopen(filename, mode, fp.release()),
        [](std::FILE* fp) { return fp ? std::fclose(fp) : 0; });
}

FILE_unique_ptr fopen_replace_before(const std::string& filename,
                                     std::string& tempfilename);
void fopen_replace_after(const std::string& filename,
                         const std::string& tempfilename, FILE_unique_ptr&& f);

class HexBedBufferFile : public HexBedBuffer {
  public:
    HexBedBufferFile(const std::string& filename);
    bufsize read(bufoffset offset, bytespan data);
    void write(WriteCallback write, const std::string& filename);
    void writeOverlay(WriteCallback write, const std::string& filename);
    void writeNew(WriteCallback write, const std::string& filename);
    void writeCopy(WriteCallback write, const std::string& filename);
    // bufsize size() noexcept;
    bufsize size() const noexcept;

  private:
    bufsize sz_;
    bufsize page_;
    bufsize pagesz_;
    FILE_unique_ptr f_;
    void updateSize();
};

};  // namespace hexbed

#endif /* HEXBED_FILE_BFILE_HH */
