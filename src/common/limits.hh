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
// common/limits.hh -- header for integer limits

#ifndef HEXBED_COMMON_LIMITS_HH
#define HEXBED_COMMON_LIMITS_HH

#include <limits>

namespace hexbed {

// clang-format off

template <typename T>
requires (std::numeric_limits<T>::is_integer
      && !std::numeric_limits<T>::is_signed)
constexpr std::size_t maxDecimalDigits() {
    return 1 + std::numeric_limits<T>::digits10;
}

template <typename T>
requires (std::numeric_limits<T>::is_integer
       && std::numeric_limits<T>::is_signed)
constexpr std::size_t maxDecimalDigits() {
    return maxDecimalDigits<std::make_unsigned_t<T>>();
}

// clang-format on

};  // namespace hexbed

#endif /* HEXBED_COMMON_LIMITS_HH */
