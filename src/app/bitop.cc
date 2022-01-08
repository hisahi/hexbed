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
// app/bitop.cc -- impl for bit operations

#include "app/bitop.hh"

#include <bit>
#include <functional>

namespace hexbed {

template <typename T>
static bool doBitwiseBinaryOp_(HexBedDocument& document, bufsize offset,
                               bufsize count, const_bytespan second,
                               const T& op) {
    return document.map(offset, count,
                        [op, second](bufoffset offset, bytespan span) -> bool {
                            bufoffset i = offset;
                            for (byte& b : span)
                                b = op(b, second[i++ % second.size()]);
                            return true;
                        });
}

template <typename T>
static bool doBitwiseUnaryOp_(HexBedDocument& document, bufsize offset,
                              bufsize count, const T& op) {
    return document.map(offset, count,
                        [op](bufoffset offset, bytespan span) -> bool {
                            for (byte& b : span) b = op(b);
                            return true;
                        });
}

template <typename T>
static bool doBitwiseShiftOp_(HexBedDocument& document, bufsize offset,
                              bufsize count, unsigned sc, const T& op) {
    return document.map(offset, count,
                        [op, sc](bufoffset offset, bytespan span) -> bool {
                            for (byte& b : span) b = op(b, sc);
                            return true;
                        });
}

template <typename T, typename Ts>
struct ShiftLeftOp {
    T operator()(const T& v, const Ts& s) const noexcept { return v << s; }
};

template <typename T, typename Ts>
struct ShiftRightOp {
    T operator()(const T& v, const Ts& s) const noexcept { return v >> s; }
};

template <typename T, typename Ts>
struct ShiftRightArithmOp {
    T operator()(const T& v, const Ts& s) const noexcept {
        return v & std::bit_floor<T>(std::numeric_limits<T>::max())
                   ? ((v >> s) | ~(T(~0) >> s))
                   : v;
    }
};

template <typename T, typename Ts>
struct RotateLeftOp {
    T operator()(const T& v, const Ts& s) const noexcept {
        return std::rotl(v, s);
    }
};

template <typename T, typename Ts>
struct RotateRightOp {
    T operator()(const T& v, const Ts& s) const noexcept {
        return std::rotr(v, s);
    }
};

template <typename T>
static constexpr T loNibbleMask;

template <>
constexpr byte loNibbleMask<byte> = 0x0F;

template <typename T>
static constexpr T hiNibbleMask = loNibbleMask<T> << 4;

template <typename T>
struct NibbleSwapOp {
    T operator()(const T& v) const noexcept {
        return (v & hiNibbleMask<T>) >> 4 | (v & loNibbleMask<T>) << 4;
    }
};

template <typename T>
struct BitReverseOp {};

template <>
struct BitReverseOp<byte> {
    byte operator()(const byte& v_) const noexcept {
        byte v = v_;
        v = (v & 0xF0) >> 4 | (v & 0x0F) << 4;
        v = (v & 0xCC) >> 2 | (v & 0x33) << 2;
        v = (v & 0xAA) >> 1 | (v & 0x55) << 1;
        return v;
    }
};

bool doBitwiseBinaryOp(HexBedDocument& document, bufsize offset, bufsize count,
                       const_bytespan second, BitwiseBinaryOp op) {
    using enum BitwiseBinaryOp;
    switch (op) {
    case Add:
        return doBitwiseBinaryOp_(document, offset, count, second,
                                  std::plus<byte>());
    case And:
        return doBitwiseBinaryOp_(document, offset, count, second,
                                  std::bit_and<byte>());
    case Or:
        return doBitwiseBinaryOp_(document, offset, count, second,
                                  std::bit_or<byte>());
    case Xor:
        return doBitwiseBinaryOp_(document, offset, count, second,
                                  std::bit_xor<byte>());
    default:
        return false;
    }
}

bool doBitwiseUnaryOp(HexBedDocument& document, bufsize offset, bufsize count,
                      BitwiseUnaryOp op) {
    using enum BitwiseUnaryOp;
    switch (op) {
    case Not:
        return doBitwiseUnaryOp_(document, offset, count, std::bit_not<byte>());
    case NibbleSwap:
        return doBitwiseUnaryOp_(document, offset, count, NibbleSwapOp<byte>());
    case BitReverse:
        return doBitwiseUnaryOp_(document, offset, count, BitReverseOp<byte>());
    default:
        return false;
    }
    return true;
}

bool doBitwiseShiftOp(HexBedDocument& document, bufsize offset, bufsize count,
                      unsigned sc, BitwiseShiftOp op) {
    using enum BitwiseShiftOp;
    switch (op) {
    case ShiftLeft:
        return doBitwiseShiftOp_(document, offset, count, sc,
                                 ShiftLeftOp<byte, unsigned>());
    case ShiftRight:
        return doBitwiseShiftOp_(document, offset, count, sc,
                                 ShiftRightOp<byte, unsigned>());
    case ShiftRightArithmetic:
        return doBitwiseShiftOp_(document, offset, count, sc,
                                 ShiftRightArithmOp<byte, unsigned>());
    case RotateLeft:
        return doBitwiseShiftOp_(document, offset, count, sc,
                                 RotateLeftOp<byte, unsigned>());
    case RotateRight:
        return doBitwiseShiftOp_(document, offset, count, sc,
                                 RotateRightOp<byte, unsigned>());
    default:
        return false;
    }
    return true;
}

static void byteSwap(byte* data, unsigned wordSize) {
    for (unsigned i = 0, e = wordSize >> 1; i < e; ++i)
        std::swap(data[i], data[wordSize - i - 1]);
}

static void byteSwap(byte* data, bufsize size, unsigned wordSize) {
    byte* end = data + size - (wordSize + 1);
    while (data < end) {
        byteSwap(data, wordSize);
        data += wordSize;
    }
}

bool doByteSwapOpNaive(HexBedDocument& document, bufsize offset, bufsize count,
                       unsigned wordSize) {
    if (count >= wordSize) {
        count -= count % wordSize;
        return document.map(
            offset, count,
            [wordSize](bufoffset offset, bytespan span) -> bool {
                byteSwap(span.data(), span.size(), wordSize);
                return true;
            },
            wordSize);
    }
    return false;
}

template <typename T>
static constexpr T byteswap(const T&);

#ifdef UINT8_MAX
template <>
constexpr std::uint8_t byteswap<std::uint8_t>(const std::uint8_t& v) {
    return v;
}
#endif

#ifdef UINT16_MAX
template <>
constexpr std::uint16_t byteswap<std::uint16_t>(const std::uint16_t& vv) {
    std::uint16_t v = vv;
    v = (v & 0xFF00U) >> 8 | (v & 0x00FFU) << 8;
    return v;
}

static void byteSwapFast2(byte* data, bufsize size) {
    auto align = reinterpret_cast<std::uint16_t*>(data);
    size >>= 1;
    for (bufsize i = 0; i < size; ++i) align[i] = byteswap(align[i]);
}

bool doByteSwapOpFast2(HexBedDocument& document, bufsize offset,
                       bufsize count) {
    count &= ~1;
    if (count) {
        return document.map(
            offset, count,
            [](bufoffset offset, bytespan span) -> bool {
                byteSwapFast2(span.data(), span.size());
                return true;
            },
            2);
    }
    return false;
}
#endif
#ifdef UINT32_MAX
template <>
constexpr std::uint32_t byteswap<std::uint32_t>(const std::uint32_t& vv) {
    std::uint32_t v = vv;
    v = (v & 0xFFFF0000UL) >> 16 | (v & 0x0000FFFFUL) << 16;
    v = (v & 0xFF00FF00UL) >> 8 | (v & 0x00FF00FFUL) << 8;
    return v;
}

static void byteSwapFast4(byte* data, bufsize size) {
    auto align = reinterpret_cast<std::uint32_t*>(data);
    size >>= 2;
    for (bufsize i = 0; i < size; ++i) align[i] = byteswap(align[i]);
}

bool doByteSwapOpFast4(HexBedDocument& document, bufsize offset,
                       bufsize count) {
    count &= ~3;
    if (count) {
        return document.map(
            offset, count,
            [](bufoffset offset, bytespan span) -> bool {
                byteSwapFast4(span.data(), span.size());
                return true;
            },
            4);
    }
    return false;
}
#endif
#ifdef UINT64_MAX
template <>
constexpr std::uint64_t byteswap<std::uint64_t>(const std::uint64_t& vv) {
    std::uint64_t v = vv;
    v = (v & 0xFFFFFFFF00000000ULL) >> 32 | (v & 0x00000000FFFFFFFFULL) << 32;
    v = (v & 0xFFFF0000FFFF0000ULL) >> 16 | (v & 0x0000FFFF0000FFFFULL) << 16;
    v = (v & 0xFF00FF00FF00FF00ULL) >> 8 | (v & 0x00FF00FF00FF00FFULL) << 8;
    return v;
}

static void byteSwapFast8(byte* data, bufsize size) {
    auto align = reinterpret_cast<std::uint64_t*>(data);
    size >>= 3;
    for (bufsize i = 0; i < size; ++i) align[i] = byteswap(align[i]);
}

bool doByteSwapOpFast8(HexBedDocument& document, bufsize offset,
                       bufsize count) {
    count &= ~7;
    if (count) {
        return document.map(
            offset, count,
            [](bufoffset offset, bytespan span) -> bool {
                byteSwapFast8(span.data(), span.size());
                return true;
            },
            8);
    }
    return false;
}

static void byteSwapFast16(byte* data, bufsize size) {
    auto align = reinterpret_cast<std::uint64_t*>(data);
    size >>= 3;
    for (bufsize i = 0; i < size; i += 2) {
        std::uint64_t tmp = byteswap(align[i]);
        align[i] = byteswap(align[i + 1]);
        align[i + 1] = tmp;
    }
}

bool doByteSwapOpFast16(HexBedDocument& document, bufsize offset,
                        bufsize count) {
    count &= ~15;
    if (count) {
        return document.map(
            offset, count,
            [](bufoffset offset, bytespan span) -> bool {
                byteSwapFast16(span.data(), span.size());
                return true;
            },
            16);
    }
    return false;
}
#endif

};  // namespace hexbed
