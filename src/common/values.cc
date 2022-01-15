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
// common/values.cc -- common constant value definitions

#include "common/values.hh"

namespace hexbed {

const char* const HEX_UPPERCASE = "0123456789ABCDEF";
const char* const HEX_LOWERCASE = "0123456789abcdef";
const strchar* const HEX_UPPERCASE_X = STRING("0123456789ABCDEF");
const strchar* const HEX_LOWERCASE_X = STRING("0123456789abcdef");

};  // namespace hexbed
