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
// common/intconv.hh -- header for integer conversions

#ifndef HEXBED_COMMON_INTCONV_HH
#define HEXBED_COMMON_INTCONV_HH

#include <bit>
#include <cctype>
#include <climits>
#include <cstdint>
#include <limits>

#include "common/hexconv.hh"
#include "common/limits.hh"
#include "common/logger.hh"
#include "common/memory.hh"
#include "common/types.hh"
#include "common/values.hh"

namespace hexbed {

template <typename T>
constexpr bool isUnpaddedType =
    std::bit_width<std::make_unsigned_t<T>>(
        std::numeric_limits<std::make_unsigned_t<T>>::max()) == CHAR_BIT *
                                                                    sizeof(T);

// clang-format off
// until it supports C++20 stuff

template <typename T>
requires (std::numeric_limits<T>::is_integer
      && !std::numeric_limits<T>::is_signed
      && isUnpaddedType<T>)
T uintFromBytes(bufsize data_n, const byte* data, bool littleEndian) {
    std::size_t n = sizeof(T);
    HEXBED_ASSERT(data_n >= n);
    T v = 0;
    for (std::size_t i = 0; i < n; ++i)
        v |= static_cast<T>(data[i])
             << (CHAR_BIT * (littleEndian ? i : n - i - 1));
    return v;
};

template <typename T>
requires (std::numeric_limits<T>::is_integer
      && std::numeric_limits<T>::is_signed
      && isUnpaddedType<T>)
T intFromBytes(bufsize data_n, const byte* data, bool littleEndian) {
    using U = std::make_unsigned_t<T>;
    static_assert(~0 == -1);
    auto v = uintFromBytes<U>(data_n, data, littleEndian);
    if (v > static_cast<U>(std::numeric_limits<T>::max()))
        return -static_cast<T>(-v);
    else
        return static_cast<T>(v);
};

template <typename T>
requires (std::numeric_limits<T>::is_integer
      && !std::numeric_limits<T>::is_signed
      && isUnpaddedType<T>)
std::size_t uintToBytes(bufsize outdata_n, byte* outdata, T result, bool littleEndian) {
    std::size_t n = sizeof(T);
    HEXBED_ASSERT(outdata_n >= n);
    for (std::size_t i = 0; i < n; ++i)
        outdata[i] |= static_cast<byte>(result
                        >> (CHAR_BIT * (littleEndian ? i : n - i - 1)));
    return n;
};

template <typename T>
requires (std::numeric_limits<T>::is_integer
      && std::numeric_limits<T>::is_signed
      && isUnpaddedType<T>)
std::size_t intToBytes(bufsize outdata_n, byte* outdata, T result, bool littleEndian) {
    return uintToBytes<std::make_unsigned_t<T>>(outdata_n, outdata,
           static_cast<std::make_unsigned_t<T>>(result), littleEndian);
};

template <typename T, std::size_t B = 10>
requires (std::numeric_limits<T>::is_integer
      && !std::numeric_limits<T>::is_signed
      && isUnpaddedType<T>)
bool uintFromString(T& result, const char* instr) {
    while (std::isspace(*instr)) ++instr;
    if (hexDigitToNum(*instr) < 0)
        return false;
    T cur = 0, prev = 0;
    static_assert(B >= 2 && B <= 16);
    while (*instr) {
        int d = hexDigitToNum(*instr);
        if (d < 0) {
            while (std::isspace(*instr)) ++instr;
            if (*instr) return false;
            break;
        }
        if (d >= static_cast<int>(B)) return false;
        prev = cur;
        cur = cur * B + d;
        if (cur < prev) return false;
        ++instr;
    }
    result = cur;
    return true;
}

template <typename T, std::size_t B = 10>
requires (std::numeric_limits<T>::is_integer
      && std::numeric_limits<T>::is_signed
      && isUnpaddedType<T>)
bool intFromString(T& result, const char* instr) {
    using U = std::make_unsigned_t<T>;
    static_assert(~0 == -1);
    while (std::isspace(*instr)) ++instr;
    bool neg = false;
    switch (*instr) {
    case '-':
        neg = true;
        [[fallthrough]];
    case '+':
        ++instr;
        break;
    default:
        if (hexDigitToNum(*instr) < 0)
            return false;
    }
    U u;
    if (!uintFromString<U, B>(u, instr))
        return false;
    if (neg) {
        if (u > static_cast<U>(-std::numeric_limits<T>::min()))
            return false;
        result = -static_cast<T>(-u);
        return true;
    } else {
        if (u > std::numeric_limits<T>::max())
            return false;
        result = static_cast<T>(u);
        return true;
    }
}

template <typename T, std::size_t B = 10>
requires (std::numeric_limits<T>::is_integer
      && !std::numeric_limits<T>::is_signed
      && isUnpaddedType<T>)
void uintToString(std::size_t outstr_n, char* outstr, T v) {
    HEXBED_ASSERT(outstr_n >= 2);
    static_assert(B >= 2 && B <= 16);
    char* term = outstr + outstr_n - 1;
    char* p = outstr;
    while (p < term && (v || p == outstr)) {
        *p++ = HEX_UPPERCASE[v % B];
        v /= B;
    }
    memReverse(reinterpret_cast<byte*>(outstr), p - outstr);
    *p++ = 0;
}

template <typename T, std::size_t B = 10>
requires (std::numeric_limits<T>::is_integer
      && std::numeric_limits<T>::is_signed
      && isUnpaddedType<T>)
void intToString(std::size_t outstr_n, char* outstr, T v) {
    using U = std::make_unsigned_t<T>;
    HEXBED_ASSERT(outstr_n >= 3);
    static_assert(B >= 2 && B <= 16);
    U u;
    if (v < 0) {
        --outstr_n;
        *outstr++ = '-';
        u = static_cast<U>(-v);
    } else
        u = static_cast<U>(v);
    uintToString<U, B>(outstr_n, outstr, u);
}

// clang-format on

};  // namespace hexbed

#endif /* HEXBED_COMMON_INTCONV_HH */
