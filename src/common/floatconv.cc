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
// common/floatconv.cc -- impl for integer <-> floating-point conversions

#include "common/floatconv.hh"

#include <bit>

namespace hexbed {

template <typename T>
static constexpr T byteswap(const T&);

template <>
constexpr std::uint32_t byteswap<std::uint32_t>(const std::uint32_t& vv) {
    std::uint32_t v = vv;
    v = (v & 0xFFFF0000UL) >> 16 | (v & 0x0000FFFFUL) << 16;
    v = (v & 0xFF00FF00UL) >> 8 | (v & 0x00FF00FFUL) << 8;
    return v;
}

template <>
constexpr std::uint64_t byteswap<std::uint64_t>(const std::uint64_t& vv) {
    std::uint64_t v = vv;
    v = (v & 0xFFFFFFFF00000000ULL) >> 32 | (v & 0x00000000FFFFFFFFULL) << 32;
    v = (v & 0xFFFF0000FFFF0000ULL) >> 16 | (v & 0x0000FFFF0000FFFFULL) << 16;
    v = (v & 0xFF00FF00FF00FF00ULL) >> 8 | (v & 0x00FF00FF00FF00FFULL) << 8;
    return v;
}

constexpr bool swapEndiannessFloat32 =
    std::bit_cast<float>(static_cast<std::uint32_t>(0x11223390UL)) < 0.0F;
constexpr bool swapEndiannessFloat64 =
    std::bit_cast<double>(static_cast<std::uint64_t>(0x1122334455667790ULL)) <
    0.0F;

float int32ToFloat32(std::uint32_t x) {
    static_assert(sizeof(float) == 4);
    static_assert(std::numeric_limits<float>::is_iec559);
    if constexpr (swapEndiannessFloat32)
        return std::bit_cast<float>(byteswap<std::uint32_t>(x));
    else
        return std::bit_cast<float>(x);
}

std::uint32_t float32ToInt32(float x) {
    static_assert(sizeof(float) == 4);
    static_assert(std::numeric_limits<float>::is_iec559);
    if constexpr (swapEndiannessFloat32)
        return byteswap<std::uint32_t>(std::bit_cast<std::uint32_t>(x));
    else
        return std::bit_cast<std::uint32_t>(x);
}

double int64ToFloat64(std::uint64_t x) {
    static_assert(sizeof(double) == 8);
    static_assert(std::numeric_limits<double>::is_iec559);
    if constexpr (swapEndiannessFloat64)
        return std::bit_cast<double>(byteswap<std::uint64_t>(x));
    else
        return std::bit_cast<double>(x);
}

std::uint64_t float64ToInt64(double x) {
    static_assert(sizeof(double) == 8);
    static_assert(std::numeric_limits<double>::is_iec559);
    if constexpr (swapEndiannessFloat64)
        return byteswap<std::uint64_t>(std::bit_cast<std::uint64_t>(x));
    else
        return std::bit_cast<std::uint64_t>(x);
}

};  // namespace hexbed
