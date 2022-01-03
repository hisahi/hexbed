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
// common/hexconv.cc -- impl for hex data conversions

#include "common/hexconv.hh"

#include <algorithm>
#include <cctype>

#include "common/config.hh"
#include "common/logger.hh"
#include "common/values.hh"

namespace hexbed {

static int octDigitToNum(char c) {
    // clang-format off
    switch (c) {
    case '0': return 0;
    case '1': return 1;
    case '2': return 2;
    case '3': return 3;
    case '4': return 4;
    case '5': return 5;
    case '6': return 6;
    case '7': return 7;
    default: return -1;
    }
    // clang-format on
}

static int decDigitToNum(char c) {
    // clang-format off
    switch (c) {
    case '0': return 0;
    case '1': return 1;
    case '2': return 2;
    case '3': return 3;
    case '4': return 4;
    case '5': return 5;
    case '6': return 6;
    case '7': return 7;
    case '8': return 8;
    case '9': return 9;
    default: return -1;
    }
    // clang-format on
}

int hexDigitToNum(char c) {
    // clang-format off
    switch (c) {
    case '0': return 0;
    case '1': return 1;
    case '2': return 2;
    case '3': return 3;
    case '4': return 4;
    case '5': return 5;
    case '6': return 6;
    case '7': return 7;
    case '8': return 8;
    case '9': return 9;
    case 'A': case 'a': return 10;
    case 'B': case 'b': return 11;
    case 'C': case 'c': return 12;
    case 'D': case 'd': return 13;
    case 'E': case 'e': return 14;
    case 'F': case 'f': return 15;
    default: return -1;
    }
    // clang-format on
}

void fastByteConv(char* out, const char* hex, byte b) {
    out[0] = hex[(b >> 4) & 15];
    out[1] = hex[b & 15];
}

std::string hexFromBytes(bufsize len, const byte* data, bool upper, bool cont) {
    const char* hex = upper ? HEX_UPPERCASE : HEX_LOWERCASE;
    char hexbuf[4] = " 00";
    unsigned offset = cont ? 0 : 1;
    std::string hexstr = "";

    for (bufsize j = 0; j < len; ++j) {
        fastByteConv(&hexbuf[1], hex, data[j]);
        hexstr += &hexbuf[offset];
        offset = 0;
    }

    return hexstr;
}

bool hexToBytes(bufsize& len, byte* data, const std::string& text) {
    bufsize l = 0, ll = len;
    byte next;
    bool nibble = false;
    for (char c : text) {
        if (isspace(c)) continue;
        int v = hexDigitToNum(c);
        if (v < 0) return false;
        if (!nibble)
            next = v << 4;
        else {
            if (data) *data++ = next | v;
            if (++l == ll) break;
        }
        nibble = !nibble;
    }
    if (nibble && data) *data++ = next, ++l;
    len = l;
    return true;
}

bool convertBaseFrom(bufsize& out, std::string_view text, unsigned base) {
    bufsize o = 0, oo = 0;
    if (text.empty()) return false;
    switch (base) {
    case 8:
        for (char c : text) {
            int i = octDigitToNum(c);
            if (i < 0) return false;
            o = (o << 3) + i;
            if (o < oo) return false;
            oo = o;
        }
        break;
    case 10:
        for (char c : text) {
            int i = decDigitToNum(c);
            if (i < 0) return false;
            o = 10 * o + i;
            if (o < oo) return false;
            oo = o;
        }
        break;
    case 16:
        for (char c : text) {
            int i = hexDigitToNum(c);
            if (i < 0) return false;
            o = (o << 4) + i;
            if (o < oo) return false;
            oo = o;
        }
        break;
    default:
        HEXBED_ASSERT(0);
        return false;
    }
    out = o;
    return true;
}

std::string convertBaseTo(bufsize in, unsigned base, bool upper) {
    if (!in) return "0";
    std::string s;
    const char* hex = upper ? HEX_UPPERCASE : HEX_LOWERCASE;
    switch (base) {
    case 8:
        while (in) {
            s += hex[in & 7];
            in >>= 3;
        }
        break;
    case 10:
        while (in) {
            s += hex[in % 10];
            in /= 10;
        }
        break;
    case 16:
        while (in) {
            s += hex[in & 15];
            in >>= 4;
        }
        break;
    default:
        HEXBED_ASSERT(0);
    }
    std::reverse(s.begin(), s.end());
    return s;
}

bool convertBaseFromNeg(bufsize& out, int& neg, std::string_view text,
                        unsigned base) {
    switch (text[0]) {
    case '-':
        neg = -1;
        text.remove_prefix(1);
        break;
    case '+':
        neg = 1;
        text.remove_prefix(1);
        break;
    default:
        neg = 0;
    }
    return convertBaseFrom(out, text, base);
}

std::string convertBaseToNeg(bufsize in, int neg, unsigned base, bool upper) {
    std::string s = convertBaseTo(in, base, upper);
    switch (neg) {
    case -1:
        return '-' + s;
    case 1:
        return '+' + s;
    default:
        return s;
    }
}

};  // namespace hexbed
