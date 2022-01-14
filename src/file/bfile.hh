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

#include "file/context.hh"
#include "file/document.hh"

namespace hexbed {

using FILE_unique_ptr =
    std::unique_ptr<std::FILE, std::function<int(std::FILE*)>>;

std::filesystem::path getBackupFilename(std::filesystem::path fn);
void makeBackupOf(HexBedContext& ctx, const std::filesystem::path& fn);

typedef std::basic_string<std::filesystem::path::value_type> pathstring;

FILE_unique_ptr fopen_unique(const char* filename, const char* mode);
FILE_unique_ptr fopen_unique(const pathstring& filename, const char* mode);
FILE_unique_ptr fopen_unique(const std::filesystem::path& filename,
                             const char* mode);

FILE_unique_ptr freopen_unique(const char* filename, const char* mode,
                               FILE_unique_ptr& fp);
FILE_unique_ptr freopen_unique(const pathstring& filename, const char* mode,
                               FILE_unique_ptr& fp);
FILE_unique_ptr freopen_unique(const std::filesystem::path& filename,
                               const char* mode, FILE_unique_ptr& fp);

FILE_unique_ptr fopen_replace_before(const std::filesystem::path& filename,
                                     std::filesystem::path& tempfilename,
                                     bool backup);
void fopen_replace_after(const std::filesystem::path& filename,
                         const std::filesystem::path& tempfilename, bool backup,
                         FILE_unique_ptr&& f);

class HexBedBufferFile : public HexBedBuffer {
  public:
    HexBedBufferFile(const std::filesystem::path& filename);
    bufsize read(bufoffset offset, bytespan data);
    void write(HexBedContext& ctx, WriteCallback write,
               const std::filesystem::path& filename);
    void writeOverlay(HexBedContext& ctx, WriteCallback write,
                      const std::filesystem::path& filename);
    void writeNew(HexBedContext& ctx, WriteCallback write,
                  const std::filesystem::path& filename);
    void writeCopy(HexBedContext& ctx, WriteCallback write,
                   const std::filesystem::path& filename);
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
