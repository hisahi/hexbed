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
// common/types.hh -- header for common types

#ifndef HEXBED_COMMON_TYPES_HH
#define HEXBED_COMMON_TYPES_HH

#include <cstdint>
#include <limits>
#include <span>
#include <string>
#include <string_view>

namespace hexbed {

#define USE_WIDE_STRINGS 0

typedef std::uint8_t byte;
typedef std::span<byte> bytespan;
typedef std::span<byte const> const_bytespan;
typedef byte* byteptr;
typedef const byte* const_byteptr;
typedef long long bufdiff;
typedef unsigned long long bufsize;
typedef bufsize bufoffset;

#if USE_WIDE_STRINGS
typedef wchar_t strchar;
typedef std::basic_string<strchar> string;
typedef std::basic_string_view<strchar> stringview;
#define CHAR(c) L##c
#define STRING(s) L##s
#define FMT_CHR "lc"
#define FMT_STR "ls"
#else
typedef char strchar;
typedef std::basic_string<strchar> string;
typedef std::basic_string_view<strchar> stringview;
#define CHAR(c) c
#define STRING(s) s
#define FMT_CHR "c"
#define FMT_STR "s"
#endif

constexpr bufsize BUFSIZE_MAX = std::numeric_limits<bufsize>::max();

};  // namespace hexbed

#endif /* HEXBED_COMMON_TYPES_HH */
