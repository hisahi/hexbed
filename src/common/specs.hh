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
// common/specs.hh -- header for compiler-dependent specifiers

#ifndef HEXBED_COMMON_SPECS_HH
#define HEXBED_COMMON_SPECS_HH

#if defined(__GNUC__)
#define HEXBED_INLINE inline
#define HEXBED_UNREACHABLE() __builtin_unreachable()
#define HEXBED_LIKELY(x) (__builtin_expect(!!(x), 1))
#define HEXBED_UNLIKELY(x) (__builtin_expect(!!(x), 0))
#else
#define HEXBED_INLINE
#define HEXBED_UNREACHABLE()
#define HEXBED_LIKELY(x) (x)
#define HEXBED_UNLIKELY(x) (x)
#endif

#endif /* HEXBED_COMMON_SPECS_HH */
