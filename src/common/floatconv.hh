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
// common/floatconv.hh -- header for integer <-> floating-point conversions

#ifndef HEXBED_COMMON_FLOATCONV_HH
#define HEXBED_COMMON_FLOATCONV_HH

#include <cfloat>
#include <climits>
#include <cstdint>
#include <limits>

#include "common/types.hh"

namespace hexbed {

float int32ToFloat32(std::uint32_t x);
std::uint32_t float32ToInt32(float x);
double int64ToFloat64(std::uint64_t x);
std::uint64_t float64ToInt64(double x);

};  // namespace hexbed

#endif /* HEXBED_COMMON_FLOATCONV_HH */
