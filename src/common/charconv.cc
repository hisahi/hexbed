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
#include <string>
#include <unordered_map>

#include "common/buffer.hh"
#include "common/intconv.hh"
#include "common/logger.hh"
#include "common/memory.hh"
#include "common/values.hh"

namespace hexbed {

static constexpr bool isInRange(char32_t c, char32_t a, char32_t b) {
    return a <= c && c <= b;
}

SingleByteCharacterSet::SingleByteCharacterSet() : map_{} { initPrint(); }

SingleByteCharacterSet::SingleByteCharacterSet(std::array<char32_t, 256> map)
    : map_(map) {
    initPrint();
}

SingleByteCharacterSet::SingleByteCharacterSet(std::u32string str) : map_{} {
    unsigned off = 0;
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

using namespace std::string_literals;

std::u32string ascii =
    U"\x00\x01\x02\x03\x04\x05\x06\x07"
    U"\x08\x09\x0a\x0b\x0c\x0d\x0e\x0f"
    U"\x10\x11\x12\x13\x14\x15\x16\x17"
    U"\x18\x19\x1a\x1b\x1c\x1d\x1e\x1f"
    U" !\"#$%&'()*+,-./"
    U"0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`"
    U"abcdefghijklmnopqrstuvwxyz{|}~"s;

std::u32string latin1 =
    U"\x00\x01\x02\x03\x04\x05\x06\x07"
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
    U"\xf0\xf1\xf2\xf3\xf4\xf5\xf6\xf7\xf8\xf9\xfa\xfb\xfc\xfd\xfe\xff"s;

#define HAS_CHARSETS_INC 1

#if HAS_CHARSETS_INC
#define CHARSET_DEFINE_S(name) std::u32string name
#define CHARSET_DEFINE_A(name) \
    std::u32string name = std::initializer_list<char32_t>
#include "common/charsets.inc"

#define CHARSETS_EXTRA_INCLUDE_SET(x) {STRING(#x), x},
static std::unordered_map<string, std::u32string> sbcsTable = {
    {STRING("ascii"), ascii},
    {STRING("latin1"), latin1},
    CHARSETS_EXTRA_INCLUDE()};
#else
static std::unordered_map<string, std::u32string> sbcsTable = {
    {STRING("ascii"), ascii}, {STRING("latin1"), latin1}};
#endif

SingleByteCharacterSet getBuiltinSbcsByName(const string& name) {
    auto s = sbcsTable.find(name);
    if (s == sbcsTable.end()) {
        LOG_WARN("unrecognized SBCS encoding name '%" FMT_STR "'", name);
        return SingleByteCharacterSet(ascii);
    }
    return SingleByteCharacterSet(s->second);
}

CharEncodeStatus mbcsEncodeNull(CharEncodeInputFunction,
                                CharEncodeOutputFunction) {
    return CharEncodeStatus{};
};

CharDecodeStatus mbcsDecodeNull(CharDecodeInputFunction,
                                CharDecodeOutputFunction) {
    return CharDecodeStatus{};
};

static MultiByteCharacterSet mbcs_null{mbcsEncodeNull, mbcsDecodeNull};

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
static int decodeCharUTF8(const byte* p, std::size_t n, char32_t& u,
                          std::size_t& rc) {
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
    if (t > UCHAR32_MAX || (0xD800UL <= t && t <= 0xDFFFUL)) return -1;
    u = t;
    return 0;
}

CharEncodeStatus mbcsEncodeUTF8(CharEncodeInputFunction input,
                                CharEncodeOutputFunction output) {
    constexpr std::size_t ubuf_n = BUFFER_SIZE / sizeof(char32_t);
    char32_t ubuf[ubuf_n];
    byte buf[4];
    bufsize n, n_in = 0, n_out = 0;
    while ((n = input(u32span{ubuf, ubuf_n}))) {
        n_in += n;
        for (std::size_t i = 0; i < n; ++i) {
            std::size_t c = encodeCharUTF8(buf, ubuf[i]);
            n_out += c;
            output(const_bytespan{buf, c});
        }
        if (n < ubuf_n) break;
    }
    return CharEncodeStatus{true, n_in, n_out};
}

CharDecodeStatus mbcsDecodeUTF8(CharDecodeInputFunction input,
                                CharDecodeOutputFunction output) {
    byte buf[BUFFER_SIZE];
    char32_t ubuf[BUFFER_SIZE / sizeof(char32_t)];
    bufsize n, n_in = 0, n_out = 0, n_outu = 0;
    std::size_t lo = 0;
    while ((n = lo + input(bytespan{buf + lo, sizeof(buf) - lo}))) {
        const byte* si = buf;
        int result;
        char32_t u;
        std::size_t q = n, rc;
        while (q && 0 == (result = decodeCharUTF8(si, q, u, rc))) {
            ubuf[n_outu++] = u;
            n_in += rc;
            si += rc;
            q -= rc;
        }
        if (n_outu) {
            n_out += n_outu;
            output(const_u32span{ubuf, n_outu});
            n_outu = 0;
        }
        if (result < 0) return CharDecodeStatus{false, n_in, n_out};
        if (result > 0 && q) {
            lo = q;
            memCopy(buf, buf + n - lo, lo);
        } else
            lo = 0;
    }
    return CharDecodeStatus{true, n_in, n_out};
}

template <bool LittleEndian>
CharEncodeStatus mbcsEncodeUTF16(CharEncodeInputFunction input,
                                 CharEncodeOutputFunction output) {
    constexpr std::size_t ubuf_n = BUFFER_SIZE / sizeof(char32_t);
    char32_t ubuf[ubuf_n];
    byte buf[4];
    bufsize n, n_in = 0, n_out = 0;
    while ((n = input(u32span{ubuf, ubuf_n}))) {
        n_in += n;
        for (std::size_t i = 0; i < n; ++i) {
            char32_t u = ubuf[i];
            if (u >= 0x10000) {
                u -= 0x10000;
                uintToBytes<std::uint16_t>(
                    2, buf,
                    static_cast<std::uint16_t>(0xD800UL |
                                               ((u >> 10) & 0x3FFUL)),
                    LittleEndian);
                uintToBytes<std::uint16_t>(
                    2, buf + 2,
                    static_cast<std::uint16_t>(0xDC00UL | (u & 0x3FFUL)),
                    LittleEndian);
                n_out += 4;
                output(const_bytespan{buf, 4});
            } else {
                uintToBytes<std::uint16_t>(
                    2, buf, static_cast<std::uint16_t>(u), LittleEndian);
                n_out += 2;
                output(const_bytespan{buf, 2});
            }
        }
        if (n < ubuf_n) break;
    }
    return CharEncodeStatus{true, n_in, n_out};
}

template <bool LittleEndian>
CharEncodeStatus mbcsEncodeUTF32(CharEncodeInputFunction input,
                                 CharEncodeOutputFunction output) {
    constexpr std::size_t ubuf_n = BUFFER_SIZE / sizeof(char32_t);
    char32_t ubuf[ubuf_n];
    byte buf[4];
    bufsize n, n_in = 0, n_out = 0;
    while ((n = input(u32span{ubuf, ubuf_n}))) {
        n_in += n;
        for (std::size_t i = 0; i < n; ++i) {
            uintToBytes<std::uint32_t>(
                4, buf, static_cast<std::uint32_t>(ubuf[i]), LittleEndian);
            n_out += 4;
            output(const_bytespan{buf, 4});
        }
        if (n < ubuf_n) break;
    }
    return CharEncodeStatus{true, n_in, n_out};
}

template <bool LittleEndian>
CharDecodeStatus mbcsDecodeUTF16(CharDecodeInputFunction input,
                                 CharDecodeOutputFunction output) {
    constexpr std::size_t buf_n = BUFFER_SIZE & ~1;
    byte buf[buf_n];
    char32_t ubuf[buf_n / 2];
    bufsize n, n_in = 0, n_out = 0, n_outu = 0;
    while ((n = input(bytespan{buf, buf_n}))) {
        n = n & ~1;
        std::size_t i = 0;
        while (i < n) {
            char32_t u = uintFromBytes<std::uint16_t>(2, &buf[i], LittleEndian);
            if ((u & 0xD800U) == 0xD800U) {
                if ((u & 0xDC00U) == 0xD800U && i + 2 < n) {
                    u = (u & 0x3FFUL) << 10;
                    i += 2;
                    char32_t u2 =
                        uintFromBytes<std::uint16_t>(2, &buf[i], LittleEndian);
                    if ((u2 & 0xDC00U) == 0xDC00U) {
                        u += (u2 & 0x3FFUL) + 0x10000UL;
                    } else
                        u = 0xFFFDU;
                } else
                    u = 0xFFFDU;
            }
            ubuf[n_outu++] = u;
            i += 2;
        }
        n_in += i;
        n_out += n_outu;
        output(const_u32span{ubuf, n_outu});
        n_outu = 0;
        if (n < buf_n) break;
    }
    return CharDecodeStatus{true, n_in, n_out};
}

template <bool LittleEndian>
CharDecodeStatus mbcsDecodeUTF32(CharDecodeInputFunction input,
                                 CharDecodeOutputFunction output) {
    constexpr std::size_t buf_n = BUFFER_SIZE & ~3;
    byte buf[buf_n];
    char32_t ubuf[buf_n / 4];
    bufsize n, n_in = 0, n_out = 0, n_outu = 0;
    while ((n = input(bytespan{buf, buf_n}))) {
        n = n & ~3;
        std::size_t i = 0;
        while (i < n) {
            char32_t u = uintFromBytes<std::uint32_t>(4, &buf[i], LittleEndian);
            if (u > UCHAR32_MAX) return CharDecodeStatus{false, n_in, n_out};
            ubuf[n_outu++] = u;
            i += 4;
        }
        n_in += i;
        n_out += n_outu;
        output(const_u32span{ubuf, n_outu});
        n_outu = 0;
        if (n < buf_n) break;
    }
    return CharDecodeStatus{true, n_in, n_out};
}

CharEncodeStatus mbcsEncodeUTF16LE(CharEncodeInputFunction input,
                                   CharEncodeOutputFunction output) {
    return mbcsEncodeUTF16<true>(input, output);
}

CharEncodeStatus mbcsEncodeUTF16BE(CharEncodeInputFunction input,
                                   CharEncodeOutputFunction output) {
    return mbcsEncodeUTF16<false>(input, output);
}

CharEncodeStatus mbcsEncodeUTF32LE(CharEncodeInputFunction input,
                                   CharEncodeOutputFunction output) {
    return mbcsEncodeUTF32<true>(input, output);
}

CharEncodeStatus mbcsEncodeUTF32BE(CharEncodeInputFunction input,
                                   CharEncodeOutputFunction output) {
    return mbcsEncodeUTF32<false>(input, output);
}

CharDecodeStatus mbcsDecodeUTF16LE(CharDecodeInputFunction input,
                                   CharDecodeOutputFunction output) {
    return mbcsDecodeUTF16<true>(input, output);
}

CharDecodeStatus mbcsDecodeUTF16BE(CharDecodeInputFunction input,
                                   CharDecodeOutputFunction output) {
    return mbcsDecodeUTF16<false>(input, output);
}

CharDecodeStatus mbcsDecodeUTF32LE(CharDecodeInputFunction input,
                                   CharDecodeOutputFunction output) {
    return mbcsDecodeUTF32<true>(input, output);
}

CharDecodeStatus mbcsDecodeUTF32BE(CharDecodeInputFunction input,
                                   CharDecodeOutputFunction output) {
    return mbcsDecodeUTF32<false>(input, output);
}

MultiByteCharacterSet mbcs_utf8{mbcsEncodeUTF8, mbcsDecodeUTF8};

static std::unordered_map<string, MultiByteCharacterSet> mbcsTable = {
    {STRING("m_utf8"), mbcs_utf8},
    {STRING("m_utf16le"), {mbcsEncodeUTF16LE, mbcsDecodeUTF16LE}},
    {STRING("m_utf16be"), {mbcsEncodeUTF16BE, mbcsDecodeUTF16BE}},
    {STRING("m_utf32le"), {mbcsEncodeUTF32LE, mbcsDecodeUTF32LE}},
    {STRING("m_utf32be"), {mbcsEncodeUTF32BE, mbcsDecodeUTF32BE}}};

MultiByteCharacterSet getBuiltinMbcsByName(const string& name) {
    auto s = mbcsTable.find(name);
    if (s == mbcsTable.end()) {
        LOG_WARN("unrecognized MBCS encoding name '%" FMT_STR "'", name);
        return mbcs_null;
    }
    return s->second;
}

CharacterEncoding::CharacterEncoding(const SingleByteCharacterSet& sbcs)
    : enc_(sbcs) {}
CharacterEncoding::CharacterEncoding(const MultiByteCharacterSet& mbcs)
    : enc_(mbcs) {}

CharEncodeStatus doSbcsEncode(const SingleByteCharacterSet& sbcs,
                              CharEncodeInputFunction input,
                              CharEncodeOutputFunction output) {
    constexpr std::size_t ubuf_n = BUFFER_SIZE / sizeof(char32_t);
    char32_t ubuf[ubuf_n];
    byte buf[ubuf_n];
    bufsize n, n_in = 0, n_out = 0;
    while ((n = input(u32span{ubuf, ubuf_n}))) {
        n_in += n;
        for (std::size_t i = 0; i < n; ++i) {
            int c = sbcs.fromChar(ubuf[i]);
            if (c < 0) return CharEncodeStatus{false, n_in, n_out};
            buf[i] = static_cast<byte>(c);
        }
        output(const_bytespan{buf, n});
        n_out += n;
        if (n < ubuf_n) break;
    }
    return CharEncodeStatus{true, n_in, n_out};
}

CharDecodeStatus doSbcsDecode(const SingleByteCharacterSet& sbcs,
                              CharDecodeInputFunction input,
                              CharDecodeOutputFunction output) {
    byte buf[BUFFER_SIZE];
    char32_t ubuf[sizeof(buf)];
    bufsize n, n_in = 0, n_out = 0, n_outu = 0;
    while ((n = input(bytespan{buf, sizeof(buf)}))) {
        for (std::size_t i = 0; i < n; ++i) {
            char32_t u = sbcs.toChar(buf[i]);
            ubuf[n_outu++] =
                u != CHAR32_INVALID && (u || !buf[i]) ? u : 0xFFFDU;
        }
        n_in += n;
        n_out += n_outu;
        output(const_u32span{ubuf, n_outu});
        n_outu = 0;
        if (n < sizeof(buf)) break;
    }
    return CharDecodeStatus{true, n_in, n_out};
}

CharEncodeStatus CharacterEncoding::encode(
    CharEncodeInputFunction input, CharEncodeOutputFunction output) const {
    if (std::holds_alternative<SingleByteCharacterSet>(enc_))
        return doSbcsEncode(std::get<SingleByteCharacterSet>(enc_), input,
                            output);
    else if (std::holds_alternative<MultiByteCharacterSet>(enc_))
        return std::get<MultiByteCharacterSet>(enc_).encode(input, output);
    else
        return CharEncodeStatus{};
}

CharDecodeStatus CharacterEncoding::decode(
    CharDecodeInputFunction input, CharDecodeOutputFunction output) const {
    if (std::holds_alternative<SingleByteCharacterSet>(enc_))
        return doSbcsDecode(std::get<SingleByteCharacterSet>(enc_), input,
                            output);
    else if (std::holds_alternative<MultiByteCharacterSet>(enc_))
        return std::get<MultiByteCharacterSet>(enc_).decode(input, output);
    else
        return CharDecodeStatus{};
}

CharacterEncoding getBuiltinCharacterEncodingByName(const string& name) {
    auto s = mbcsTable.find(name);
    if (s == mbcsTable.end())
        return CharacterEncoding(getBuiltinSbcsByName(name));
    return CharacterEncoding(s->second);
}

CharEncodeInputFunction charEncodeFromArray(std::size_t n,
                                            const char32_t* arr) {
    return [&n, &arr](u32span s) -> bufsize {
        std::size_t r = std::min<std::size_t>(n, s.size());
        for (std::size_t i = 0; i < r; ++i, --n) s[i] = *arr++;
        return r;
    };
}

CharEncodeInputFunction charEncodeFromString(std::u32string s) {
    std::size_t n = s.size();
    const char32_t* arr = s.data();
    return [&n, &arr](u32span s) -> bufsize {
        std::size_t r = std::min<std::size_t>(n, s.size());
        for (std::size_t i = 0; i < r; ++i, --n) s[i] = *arr++;
        return r;
    };
}

CharEncodeOutputFunction charEncodeToNull() {
    return [](const_bytespan s) {};
}

CharEncodeOutputFunction charEncodeToArray(std::size_t n, byte* arr) {
    return [&n, &arr](const_bytespan s) {
        std::size_t r = std::min<std::size_t>(n, s.size());
        for (std::size_t i = 0; i < r; ++i, --n) *arr++ = s[i];
    };
}

CharDecodeInputFunction charDecodeFromArray(std::size_t n, const byte* arr) {
    return [&n, &arr](bytespan s) -> bufsize {
        std::size_t r = std::min<std::size_t>(n, s.size());
        for (std::size_t i = 0; i < r; ++i, --n) s[i] = *arr++;
        return r;
    };
}

CharDecodeOutputFunction charDecodeToNull() {
    return [](const_u32span s) {};
}

CharDecodeOutputFunction charDecodeToStream(u32ostringstream& stream) {
    return [&stream](const_u32span s) { stream.write(s.data(), s.size()); };
}

std::u32string sbcsFromBytes(const SingleByteCharacterSet& sbcs, bufsize len,
                             const byte* data) {
    std::vector<char32_t> chars;
    for (bufsize j = 0; j < len; ++j) {
        char32_t u = sbcs.toChar(data[j]);
        if (u == CHAR32_INVALID) continue;
        chars.push_back(u);
    }
    return std::u32string(chars.begin(), chars.end());
}

bool sbcsToBytes(const SingleByteCharacterSet& sbcs, bufsize& len, byte* data,
                 const std::u32string& text) {
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

std::u32string sbcsFromBytes(bufsize len, const byte* data) {
    return sbcsFromBytes(sbcs, len, data);
}

bool sbcsToBytes(bufsize& len, byte* data, const std::u32string& text) {
    return sbcsToBytes(sbcs, len, data, text);
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

extern bool isUnicodePrintable(char32_t c);

/*
bool isUnicodePrintable(char32_t c) { return false; }
*/

bool isPrintable(char32_t c) {
    if (c == 0x7F || c == CHAR32_INVALID) return false;
    if (isInRange(c, 0x20, 0x7E) || isInRange(c, 0xA1, 0xAC) ||
        isInRange(c, 0xAE, 0xFF))
        return true;
    if (!(c & ~0xFF)) return false;
    return isUnicodePrintable(c);
}

char32_t convertCharFrom(unsigned utf, bufsize n, const byte* b,
                         bool printable) {
    switch (utf) {
    default:
    case 0:
        return sbcs.toPrintableChar(*b);
    case 1:
    case 2:
        if (n < 2)
            return 0;
        else {
            char32_t c = static_cast<char32_t>(
                uintFromBytes<std::uint16_t>(2, b, utf == 1));
            if (printable && !isPrintable(c)) c = 0;
            return c;
        }
        break;
    case 3:
    case 4:
        if (n < 4)
            return 0;
        else {
            char32_t c = static_cast<char32_t>(
                uintFromBytes<std::uint32_t>(4, b, utf == 3));
            if (c > UCHAR32_MAX) {
                c = printable ? 0 : 0xFFFDU;
            } else if (printable && !isPrintable(c))
                c = 0;
            return c;
        }
        break;
    }
}

std::u32string convertCharsFrom(unsigned utf, bufsize n, const byte* b,
                                bool printable) {
    std::u32string w;
    switch (utf) {
    case 0:
        return sbcsFromBytes(n, b);
    case 1:
    case 2:
        while (n >= 2) {
            char32_t c = convertCharFrom(utf, n, b, false);
            n -= 2, b += 2;
            if ((c & 0xD800U) == 0xD800U) {
                if ((c & 0xDC00U) == 0xD800U && n >= 2) {
                    c = (c & 0x3FFUL) << 10;
                    char32_t c2 = convertCharFrom(utf, n, b);
                    n -= 2, b += 2;
                    if ((c2 & 0xDC00U) == 0xDC00U) {
                        c += (c2 & 0x3FFUL) + 0x10000UL;
                    } else
                        c = 0xFFFDU;
                } else
                    c = 0xFFFDU;
            }
            if (!printable || isPrintable(c)) w += c;
        }
        return w;
    case 3:
    case 4:
        while (n >= 4) {
            char32_t c = convertCharFrom(utf, n, b, printable);
            n -= 4, b += 4;
            if (!printable || isPrintable(c)) w += c;
        }
        return w;
    }
    return w;
}

bool convertCharsTo(unsigned utf, bufsize& rlen, byte* data,
                    const std::u32string& text) {
    const char32_t* sp = text.data();
    bufsize len = rlen, outp = 0;
    std::size_t sn = text.size();
    bool le;
    switch (utf) {
    case 0:
        return sbcsToBytes(len, data, text);
    case 1:
    case 2:
        le = utf == 1;
        while (len >= 2 && sn) {
            std::uint32_t u = static_cast<std::uint32_t>(*sp++);
            --sn;
            if (u >= 0x10000) {
                u -= 0x10000UL;
                uintToBytes<std::uint16_t>(
                    len, data,
                    static_cast<std::uint16_t>(0xD800 | ((u >> 10) & 0x3FFU)),
                    le);
                len -= 2, data += 2, outp += 2;
                if (len >= 2) {
                    uintToBytes<std::uint16_t>(
                        len, data,
                        static_cast<std::uint16_t>(0xDC00 | (u & 0x3FFU)), le);
                    len -= 2, data += 2, outp += 2;
                } else
                    return false;
            } else {
                uintToBytes<std::uint16_t>(len, data,
                                           static_cast<std::uint16_t>(u), le);
                len -= 2, data += 2, outp += 2;
            }
        }
        rlen = outp;
        return true;
    case 3:
    case 4:
        le = utf == 3;
        while (len >= 4 && sn) {
            uintToBytes<std::uint32_t>(len, data,
                                       static_cast<std::uint32_t>(*sp++), le);
            --sn;
            len -= 4, data += 4, outp += 4;
        }
        rlen = outp;
        return true;
    }
    return false;
}

};  // namespace hexbed
