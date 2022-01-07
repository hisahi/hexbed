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

enum class BitwiseUnaryOp { Not };

bool doBitwiseBinaryOp(HexBedDocument& document, bufsize offset, bufsize count,
                       const_bytespan second, BitwiseBinaryOp op);
bool doBitwiseUnaryOp(HexBedDocument& document, bufsize offset, bufsize count,
                      BitwiseUnaryOp op);
bool doBitwiseShiftOp(HexBedDocument& document, bufsize offset, bufsize count,
                      unsigned sc, BitwiseShiftOp op);

};  // namespace hexbed

#endif /* HEXBED_APP_BITOP_HH */
