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
// file/bnew.hh -- header for the new document buffer

#ifndef HEXBED_FILE_BNEW_HH
#define HEXBED_FILE_BNEW_HH

#include "file/context.hh"
#include "file/document.hh"

namespace hexbed {

class HexBedBufferNew : public HexBedBuffer {
  public:
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
};

};  // namespace hexbed

#endif /* HEXBED_FILE_BNEW_HH */
