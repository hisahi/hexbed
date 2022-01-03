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

static bool isPrintable(char32_t c) {
    if (!(c & 0xFFFFFF60UL) || c == 0x7F || c == CHAR32_INVALID) return false;
    return true;
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
int SingleByteCharacterSet::fromChar(char32_t u) {
    for (unsigned i = 0; i < 256; ++i)
        if (map_[i] == u) return i;
    return -1;
}

char32_t SingleByteCharacterSet::toChar(byte b) { return map_[b]; }

// only give printable characters, or 0 if not printable
char32_t SingleByteCharacterSet::toPrintableChar(byte b) {
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

static std::unordered_map<std::string, std::u32string> sbcsTable = {
    {"ascii", ascii}};

SingleByteCharacterSet getSbcsByName(const std::string& name) {
    auto s = sbcsTable.find(name);
    if (s == sbcsTable.end()) {
        LOG_WARN("unrecognized SBCS encoding name '%s'", name);
        return SingleByteCharacterSet(ascii);
    }
    return SingleByteCharacterSet(s->second);
}

std::wstring sbcsFromBytes(bufsize len, const byte* data) {
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

bool sbcsToBytes(bufsize& len, byte* data, const std::wstring& text) {
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

};  // namespace hexbed
