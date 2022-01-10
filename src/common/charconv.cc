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
// common/charconv.cc -- impl for character conversions

#include "common/charconv.hh"

#include <cctype>
#include <unordered_map>

#include "common/logger.hh"
#include "common/values.hh"

namespace hexbed {

static constexpr bool isInRange(char32_t c, char32_t a, char32_t b) {
    return a <= c && c <= b;
}

static bool isPrintable(char32_t c) {
    if (c == 0x7F || c == CHAR32_INVALID) return false;
    return isInRange(c, 0x20, 0x7E) || isInRange(c, 0xA1, 0xAC) ||
           isInRange(c, 0xAE, 0xFF);
}

SingleByteCharacterSet::SingleByteCharacterSet() : map_{} { initPrint(); }

SingleByteCharacterSet::SingleByteCharacterSet(std::array<char32_t, 256> map)
    : map_(map) {
    initPrint();
}

SingleByteCharacterSet::SingleByteCharacterSet(std::u32string str) : map_{} {
    unsigned off = 1;
    for (char32_t c : str) {
        map_[off] = c;
        if (++off >= 256) break;
    }
    initPrint();
}

void SingleByteCharacterSet::initPrint() {
    for (unsigned i = 0; i < 256; ++i) print_[i] = isPrintable(map_[i]);
}

// 0-255 or -1 for not allowed
int SingleByteCharacterSet::fromChar(char32_t u) const {
    for (unsigned i = 0; i < 256; ++i)
        if (map_[i] == u) return i;
    return -1;
}

char32_t SingleByteCharacterSet::toChar(byte b) const { return map_[b]; }

// only give printable characters, or 0 if not printable
char32_t SingleByteCharacterSet::toPrintableChar(byte b) const {
    return print_[b] ? map_[b] : 0;
}

struct charset_pair {
    unsigned o;
    std::u32string u;
};

SingleByteCharacterSet sbcs;

std::u32string ascii =
    U"\x01\x02\x03\x04\x05\x06\x07"
    U"\x08\x09\x0a\x0b\x0c\x0d\x0e\x0f"
    U"\x10\x11\x12\x13\x14\x15\x16\x17"
    U"\x18\x19\x1a\x1b\x1c\x1d\x1e\x1f"
    U" !\"#$%&'()*+,-./"
    U"0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`"
    U"abcdefghijklmnopqrstuvwxyz{|}~";

std::u32string latin1 =
    U"\x01\x02\x03\x04\x05\x06\x07"
    U"\x08\x09\x0a\x0b\x0c\x0d\x0e\x0f"
    U"\x10\x11\x12\x13\x14\x15\x16\x17"
    U"\x18\x19\x1a\x1b\x1c\x1d\x1e\x1f"
    U" !\"#$%&'()*+,-./"
    U"0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`"
    U"abcdefghijklmnopqrstuvwxyz{|}~\x7f"
    U"\x80\x81\x82\x83\x84\x85\x86\x87\x88\x89\x8a\x8b\x8c\x8d\x8e\x8f"
    U"\x90\x91\x92\x93\x94\x95\x96\x97\x98\x99\x9a\x9b\x9c\x9d\x9e\x9f"
    U"\xa0\xa1\xa2\xa3\xa4\xa5\xa6\xa7\xa8\xa9\xaa\xab\xac\xad\xae\xaf"
    U"\xb0\xb1\xb2\xb3\xb4\xb5\xb6\xb7\xb8\xb9\xba\xbb\xbc\xbd\xbe\xbf"
    U"\xc0\xc1\xc2\xc3\xc4\xc5\xc6\xc7\xc8\xc9\xca\xcb\xcc\xcd\xce\xcf"
    U"\xd0\xd1\xd2\xd3\xd4\xd5\xd6\xd7\xd8\xd9\xda\xdb\xdc\xdd\xde\xdf"
    U"\xe0\xe1\xe2\xe3\xe4\xe5\xe6\xe7\xe8\xe9\xea\xeb\xec\xed\xee\xef"
    U"\xf0\xf1\xf2\xf3\xf4\xf5\xf6\xf7\xf8\xf9\xfa\xfb\xfc\xfd\xfe\xff";

static std::unordered_map<std::string, std::u32string> sbcsTable = {
    {"ascii", ascii}, {"latin1", latin1}};

SingleByteCharacterSet getSbcsByName(const std::string& name) {
    auto s = sbcsTable.find(name);
    if (s == sbcsTable.end()) {
        LOG_WARN("unrecognized SBCS encoding name '%s'", name);
        return SingleByteCharacterSet(ascii);
    }
    return SingleByteCharacterSet(s->second);
}

std::wstring sbcsFromBytes(const SingleByteCharacterSet& sbcs, bufsize len,
                           const byte* data) {
    std::vector<wchar_t> chars;
    for (bufsize j = 0; j < len; ++j) {
        char32_t u = sbcs.toChar(data[j]);
        wchar_t w;
        if (u == CHAR32_INVALID)
            continue;
        else if (std::numeric_limits<wchar_t>::max() >= 0x110000UL ||
                 u < 0x10000)
            w = u;
        else
            w = 0xFFFDU;
        chars.push_back(w);
    }
    return std::wstring(chars.begin(), chars.end());
}

bool sbcsToBytes(const SingleByteCharacterSet& sbcs, bufsize& len, byte* data,
                 const std::wstring& text) {
    bufsize l = 0, ll = len;
    for (wchar_t c : text) {
        int v = sbcs.fromChar(c);
        if (v < 0) return false;
        if (data) *data++ = static_cast<byte>(v);
        if (++l == ll) break;
    }
    len = l;
    return true;
}

std::wstring sbcsFromBytes(bufsize len, const byte* data) {
    return sbcsFromBytes(sbcs, len, data);
}

bool sbcsToBytes(bufsize& len, byte* data, const std::wstring& text) {
    return sbcsToBytes(sbcs, len, data, text);
}

};  // namespace hexbed
