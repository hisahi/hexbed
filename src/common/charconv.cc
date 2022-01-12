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
#include <sstream>
#include <unordered_map>

#include "common/intconv.hh"
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

static std::size_t encodeCharUTF8(byte* out, char32_t c) {
    bufsize i = 0;
    if (c < 0x80UL) {
        out[i++] = static_cast<byte>(c & 0x7F);
    } else if (c < 0x800UL) {
        out[i++] = static_cast<byte>(0xC0 | ((c >> 6) & 0x1F));
        out[i++] = static_cast<byte>(0x80 | (c & 0x3F));
    } else if (c < 0x10000UL) {
        out[i++] = static_cast<byte>(0xE0 | ((c >> 12) & 0x0F));
        out[i++] = static_cast<byte>(0x80 | ((c >> 6) & 0x3F));
        out[i++] = static_cast<byte>(0x80 | (c & 0x3F));
    } else if (c < 0x110000UL) {
        out[i++] = static_cast<byte>(0xF0 | ((c >> 18) & 0x07));
        out[i++] = static_cast<byte>(0x80 | ((c >> 12) & 0x3F));
        out[i++] = static_cast<byte>(0x80 | ((c >> 6) & 0x3F));
        out[i++] = static_cast<byte>(0x80 | (c & 0x3F));
    }
    return i;
}

// 0 = OK
// > 0 = truncated
// < 0 = invalid
int decodeCharUTF8(const byte* p, std::size_t n, char32_t& u, std::size_t& rc) {
    if (!n) return 1;
    byte c = *p;
    char32_t t;
    std::size_t j;
    if (!(c & 0x80)) {
        rc = 1;
        u = c;
        return 0;
    } else if ((c & 0xE0) == 0xC0) {
        t = c & 0x1F;
        j = 2;
    } else if ((c & 0xF0) == 0xE0) {
        t = c & 0x0F;
        j = 3;
    } else if ((c & 0xF8) == 0xF0) {
        t = c & 0x07;
        j = 4;
    } else {
        return -1;
    }
    if (j > n) return 1;
    for (std::size_t i = 1; i < j; ++i) {
        c = p[i];
        if ((c & 0xC0) != 0x80) return -1;
        t = (t << 6) | (c & 0x3F);
    }
    rc = j;
    if (t >= 0x110000UL || (0xD800UL <= t && t <= 0xDFFFUL)) return -1;
    u = t;
    return 0;
}

std::size_t encodeCharMbcsOrSbcs(TextEncoding enc,
                                 const SingleByteCharacterSet& sbcs, char32_t c,
                                 std::size_t size, byte* out) {
    if (!size) return 0;
    byte outbuf[MBCS_CHAR_MAX];
    std::size_t outlen = 0;
    bool little = false;
    switch (enc) {
    case TextEncoding::SBCS: {
        int i = sbcs.fromChar(c);
        if (i >= 0) outbuf[outlen++] = i;
        break;
    }
    case TextEncoding::UTF8:
        outlen = encodeCharUTF8(outbuf, c);
        break;
    case TextEncoding::UTF16LE:
        little = true;
        [[fallthrough]];
    case TextEncoding::UTF16BE:
        if (c >= 0x10000UL) {
            c -= 0x10000UL;
            outlen += uintToBytes<std::uint16_t>(
                sizeof(outbuf), outbuf,
                static_cast<std::uint16_t>(0xD800UL | ((c >> 10) & 0x3FF)),
                little);
            outlen += uintToBytes<std::uint16_t>(
                sizeof(outbuf) - outlen, outbuf + outlen,
                static_cast<std::uint16_t>(0xDC00UL | (c & 0x3FF)), little);
        } else
            outlen = uintToBytes<std::uint16_t>(
                sizeof(outbuf), outbuf, static_cast<std::uint16_t>(c), little);
        break;
    case TextEncoding::UTF32LE:
        little = true;
        [[fallthrough]];
    case TextEncoding::UTF32BE:
        outlen = uintToBytes<std::uint32_t>(
            sizeof(outbuf), outbuf, static_cast<std::uint32_t>(c), little);
        break;
    default:
        return 0;
    }
    return memCopy(out, outbuf, std::min(outlen, size));
}

DecodeStatus decodeStringMbcsOrSbcs(TextEncoding enc,
                                    const SingleByteCharacterSet& sbcs,
                                    u32ostringstream& out,
                                    const_bytespan data) {
    DecodeStatus status{true, 0, 0};
    std::size_t ds = data.size();
    bool little = false;
    switch (enc) {
    case TextEncoding::SBCS:
        for (std::size_t i = 0; i < ds; ++i) {
            char32_t u = sbcs.toChar(data[i]);
            ++status.readCount;
            if (!u) {
                status.ok = false;
                return status;
            }
            ++status.charCount;
            out << u;
        }
        break;
    case TextEncoding::UTF8: {
        const byte* p = data.data();
        const byte* pe = data.data() + ds;
        char32_t u;
        std::size_t rc;
        while (p < pe) {
            int err = decodeCharUTF8(p, pe - p, u, rc);
            if (err) {
                status.ok = err > 0;
                return status;
            }
            status.readCount += rc;
            p += rc;
            ++status.charCount;
            out << u;
        }
        break;
    }
    case TextEncoding::UTF16LE:
        little = true;
        [[fallthrough]];
    case TextEncoding::UTF16BE: {
        const byte* p = data.data();
        --ds;
        for (std::size_t j = 0; j < ds; j += 2) {
            std::uint16_t u = uintFromBytes<std::uint16_t>(
                sizeof(std::uint16_t), &p[j], little);
            if ((u & 0xDC00) == 0xD800) {
                j += 2;
                if (j >= ds) return status;
                status.readCount += 2;
                u = (u & 0x3FF) << 10;
                u += uintFromBytes<std::uint16_t>(sizeof(std::uint16_t), &p[j],
                                                  little);
                u += 0x10000UL - 0xDC00UL;
            } else if ((u & 0xDC00) == 0xDC00) {
                status.ok = false;
                return status;
            }
            status.readCount += 2;
            ++status.charCount;
            out << static_cast<char32_t>(u);
        }
        break;
    }
    case TextEncoding::UTF32LE:
        little = true;
        [[fallthrough]];
    case TextEncoding::UTF32BE: {
        const byte* p = data.data();
        ds -= 3;
        for (std::size_t j = 0; j < ds; j += 4) {
            std::uint32_t u = uintFromBytes<std::uint32_t>(
                sizeof(std::uint32_t), &p[j], little);
            status.readCount += 4;
            ++status.charCount;
            out << static_cast<char32_t>(u);
        }
        break;
    }
    default:
        return DecodeStatus{};
    }
    return status;
}

template <std::size_t N>
static std::u32string wstringToU32string_(const std::wstring& w);

template <>
[[maybe_unused]] std::u32string wstringToU32string_<2>(const std::wstring& w) {
    // assume UTF-16
    std::basic_ostringstream<char32_t> ss;
    std::size_t sl = w.size();
    for (std::size_t i = 0; i < sl; ++i) {
        wchar_t c = w[i];
        if ((c & 0xDC00UL) == 0xD800UL) {
            char32_t u = (c & 0x3FFUL) << 10;
            c = w[++i];
            if ((c & 0xDC00UL) != 0xDC00UL) continue;
            u |= (c & 0x3FFUL);
            ss << (u + 0x10000UL);
        } else if ((c & 0xDC00UL) != 0xDC00UL)
            ss << static_cast<char32_t>(c);
    }
    return ss.str();
}

template <>
[[maybe_unused]] std::u32string wstringToU32string_<4>(const std::wstring& w) {
    // assume UTF-32
    std::basic_ostringstream<char32_t> ss;
    for (wchar_t c : w) ss << static_cast<char32_t>(c);
    return ss.str();
}

std::u32string wstringToU32string(const std::wstring& w) {
    return wstringToU32string_<sizeof(wchar_t)>(w);
}

template <std::size_t N>
static std::wstring u32stringToWstring_(const std::u32string& u);

template <>
[[maybe_unused]] std::wstring u32stringToWstring_<2>(const std::u32string& u) {
    // assume UTF-16
    std::wostringstream ss;
    std::size_t sl = u.size();
    for (std::size_t i = 0; i < sl; ++i) {
        char32_t c = u[i];
        if (c >= 0x10000UL) {
            c -= 0x10000UL;
            ss << static_cast<wchar_t>(0xD800UL | ((c >> 10) & 0x3FF));
            ss << static_cast<wchar_t>(0xDC00UL | (c & 0x3FF));
        } else
            ss << static_cast<wchar_t>(c);
    }
    return ss.str();
}

template <>
[[maybe_unused]] std::wstring u32stringToWstring_<4>(const std::u32string& u) {
    // assume UTF-32
    std::wostringstream ss;
    for (char32_t c : u) ss << static_cast<wchar_t>(c);
    return ss.str();
}

std::wstring u32stringToWstring(const std::u32string& w) {
    return u32stringToWstring_<sizeof(wchar_t)>(w);
}

};  // namespace hexbed
