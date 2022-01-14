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
// ui/clipboard.hh -- header for the clipboard functionality

#ifndef HEXBED_UI_CLIPBOARD_HH
#define HEXBED_UI_CLIPBOARD_HH

#include <wx/string.h>

#include <stdexcept>

#include "file/document.hh"

namespace hexbed {
namespace clip {

class ClipboardError : public std::runtime_error {
  public:
    inline ClipboardError() : std::runtime_error("clipboard error") {}
};

bool HasClipboard();
void CopyBytes(HexBedDocument& document, bufsize off, bufsize cnt, bool text);
bool PasteBytes(HexBedDocument& document, bool insert, bufsize off, bufsize cnt,
                bool text, bufsize& len);

};  // namespace clip
};  // namespace hexbed

#endif /* HEXBED_UI_CLIPBOARD_HH */
