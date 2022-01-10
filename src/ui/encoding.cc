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
#include "common/charconv.hh"

namespace hexbed {

template <typename T>
bool textEncode_(const T& buf, bufsize& outp, HexBedDocument* doc) {
    outp = buf.length();
    const byte* data = reinterpret_cast<const byte*>(buf.data());
    if (doc) doc->replace(0, doc->size(), const_bytespan(data, outp));
    return true;
}

bool textEncode(const std::string& encoding, const wxString& text,
                bufsize& outp, HexBedDocument* doc) {
    if (!std::strncmp(encoding.c_str(), "m_", 2)) {
        if (encoding == "m_utf8") {
            return textEncode_(text.utf8_str(), outp, doc);
        } else if (encoding == "m_utf16le") {
            return textEncode_(text.mb_str(wxMBConvUTF16LE()), outp, doc);
        } else if (encoding == "m_utf16be") {
            return textEncode_(text.mb_str(wxMBConvUTF16BE()), outp, doc);
        } else if (encoding == "m_utf32le") {
            return textEncode_(text.mb_str(wxMBConvUTF32LE()), outp, doc);
        } else if (encoding == "m_utf32be") {
            return textEncode_(text.mb_str(wxMBConvUTF32BE()), outp, doc);
        }
    }
    SingleByteCharacterSet sbcs =
        getSbcsByName(!encoding.empty() ? encoding : config().charset);
    std::wstring wstr = text.ToStdWstring();
    if (!sbcsToBytes(sbcs, outp, nullptr, wstr)) return false;
    if (doc) {
        auto buffer = std::make_unique<byte[]>(outp);
        if (!sbcsToBytes(sbcs, outp, buffer.get(), wstr)) return false;
        doc->replace(0, doc->size(), const_bytespan{buffer.get(), outp});
    }
    return true;
}

bool textDecode(const std::string& encoding, wxString& text,
                const_bytespan data) {
    if (!data.size()) {
        text = wxEmptyString;
        return true;
    }
    if (!std::strncmp(encoding.c_str(), "m_", 2)) {
        const char* c = reinterpret_cast<const char*>(data.data());
        if (encoding == "m_utf8") {
            return !(text = wxString::FromUTF8(c, data.size())).empty();
        } else if (encoding == "m_utf16le") {
            return !(text = wxString(c, wxMBConvUTF16LE(), data.size()))
                        .empty();
        } else if (encoding == "m_utf16be") {
            return !(text = wxString(c, wxMBConvUTF16BE(), data.size()))
                        .empty();
        } else if (encoding == "m_utf32le") {
            return !(text = wxString(c, wxMBConvUTF32LE(), data.size()))
                        .empty();
        } else if (encoding == "m_utf32be") {
            return !(text = wxString(c, wxMBConvUTF32BE(), data.size()))
                        .empty();
        }
    }
    SingleByteCharacterSet sbcs =
        getSbcsByName(!encoding.empty() ? encoding : config().charset);
    text = sbcsFromBytes(sbcs, data.size(), data.data());
    return true;
}

};  // namespace hexbed
