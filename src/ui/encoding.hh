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
// ui/encoding.hh -- header for multibyte encodings and list of text encodings

#ifndef HEXBED_UI_ENCODING_HH
#define HEXBED_UI_ENCODING_HH

#include <wx/string.h>

#include "common/types.hh"
#include "file/document.hh"

namespace hexbed {

bool textEncode(const std::string& encoding, const wxString& text,
                bufsize& outp, HexBedDocument* doc);
bool textDecode(const std::string& encoding, wxString& text,
                const_bytespan data);

// clang-format off

#define MBCS_ENCODING_KEYS() \
    "m_utf8",                \
    "m_utf16le",             \
    "m_utf16be",             \
    "m_utf32le",             \
    "m_utf32be"
#define MBCS_ENCODING_NAMES()                                                  \
    _("Unicode, UTF-8"),                                                       \
    _("Unicode, UTF-16LE (little-endian)"),                                    \
    _("Unicode, UTF-16BE (big-endian)"),                                       \
    _("Unicode, UTF-32LE (little-endian)"),                                    \
    _("Unicode, UTF-32BE (big-endian)")

#define SBCS_ENCODING_KEYS() \
    "ascii",                 \
    "latin1"
#define SBCS_ENCODING_NAMES()                                                  \
    _("ASCII"),                                                                \
    _("Latin-1 (ISO 8859-1)")

// clang-format on

};  // namespace hexbed

#endif /* HEXBED_UI_ENCODING_HH */
