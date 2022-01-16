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
// ui/encoding.cc -- impl for multibyte encodings

#include "ui/encoding.hh"

#include <wx/strconv.h>

#include <cstring>

#include "app/config.hh"
#include "app/encoding.hh"
#include "common/charconv.hh"

#if HAS_ICU
#include <unicode/uchar.h>
#endif

namespace hexbed {

template <typename T>
bool textEncode_(const T& buf, bufsize& outp, HexBedDocument* doc) {
    outp = buf.length();
    const byte* data = reinterpret_cast<const byte*>(buf.data());
    if (doc) doc->replace(0, doc->size(), const_bytespan(data, outp));
    return true;
}

bool textEncode(const string& encoding, const wxString& text, bufsize& outp,
                HexBedDocument* doc) {
    CharEncodeStatus status;
    bool ok = doc->pry(
        0, doc->size(),
        [&encoding, &text, &status](HexBedTask& task,
                                    std::function<void(const_bytespan)> out) {
            status = getCharacterEncodingByName(encoding).encode(
                charEncodeFromString(wstringToU32string(text.ToStdWstring())),
                out);
            if (!status.ok) task.cancel();
        });
    if (!ok) return false;
    outp = status.wroteBytes;
    return true;
}

bool textDecode(const string& encoding, wxString& text, const_bytespan data) {
    if (!data.size()) {
        text = wxEmptyString;
        return true;
    }
    u32ostringstream ss;
    CharDecodeStatus status = getCharacterEncodingByName(encoding).decode(
        charDecodeFromArray(data.size(), data.data()), charDecodeToStream(ss));
    if (!status.ok) return false;
    text = u32stringToWstring(ss.str());
    return true;
}

// implement the func needed by common/charconv.cc
bool isUnicodePrintable(char32_t c) {
#if HAS_ICU
    return u_isprint(static_cast<UChar32>(c));
#else
    return false;
#endif
}

};  // namespace hexbed
