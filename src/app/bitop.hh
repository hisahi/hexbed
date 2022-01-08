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
// app/bitop.hh -- header for bit operations

#ifndef HEXBED_APP_BITOP_HH
#define HEXBED_APP_BITOP_HH

#define __STDC_LIMIT_MACROS
#include <cstdint>

#include "common/types.hh"
#include "file/document.hh"

namespace hexbed {

enum class BitwiseBinaryOp { Add, And, Or, Xor };

enum class BitwiseShiftOp {
    ShiftLeft,
    ShiftRight,
    ShiftRightArithmetic,
    RotateLeft,
    RotateRight
};

enum class BitwiseUnaryOp { Not, NibbleSwap, BitReverse };

bool doBitwiseBinaryOp(HexBedDocument& document, bufsize offset, bufsize count,
                       const_bytespan second, BitwiseBinaryOp op);
bool doBitwiseUnaryOp(HexBedDocument& document, bufsize offset, bufsize count,
                      BitwiseUnaryOp op);
bool doBitwiseShiftOp(HexBedDocument& document, bufsize offset, bufsize count,
                      unsigned sc, BitwiseShiftOp op);
bool doByteSwapOpNaive(HexBedDocument& document, bufsize offset, bufsize count,
                       unsigned wordSize);

#ifdef UINT16_MAX
bool doByteSwapOpFast2(HexBedDocument& document, bufsize offset, bufsize count);
#endif
#ifdef UINT32_MAX
bool doByteSwapOpFast4(HexBedDocument& document, bufsize offset, bufsize count);
#endif
#ifdef UINT64_MAX
bool doByteSwapOpFast8(HexBedDocument& document, bufsize offset, bufsize count);
bool doByteSwapOpFast16(HexBedDocument& document, bufsize offset,
                        bufsize count);
#endif

template <unsigned wordSize>
bool doByteSwapOp(HexBedDocument& document, bufsize offset, bufsize count) {
#ifdef UINT16_MAX
    if constexpr (wordSize == 2)
        return doByteSwapOpFast2(document, offset, count);
#endif
#ifdef UINT32_MAX
    if constexpr (wordSize == 4)
        return doByteSwapOpFast4(document, offset, count);
#endif
#ifdef UINT64_MAX
    if constexpr (wordSize == 8)
        return doByteSwapOpFast8(document, offset, count);
    if constexpr (wordSize == 16)
        return doByteSwapOpFast16(document, offset, count);
#endif
    return doByteSwapOpNaive(document, offset, count, wordSize);
}

};  // namespace hexbed

#endif /* HEXBED_APP_BITOP_HH */
