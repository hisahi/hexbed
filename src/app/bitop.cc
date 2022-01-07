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
    T operator()(const T& v, const Ts& s) const noexcept { return v; }
};

template <typename T, typename Ts>
struct RotateLeftOp {
    T operator()(const T& v, const Ts& s) const noexcept { return v; }
};

template <typename T, typename Ts>
struct RotateRightOp {
    T operator()(const T& v, const Ts& s) const noexcept { return v; }
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

};  // namespace hexbed
