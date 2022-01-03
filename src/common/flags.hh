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
// common/flags.hh -- header for flags (controlling behavior through defines)

#ifndef HEXBED_COMMON_FLAGS_HH
#define HEXBED_COMMON_FLAGS_HH

/* use wxPreferencesEditor instead of a custom preference dialog.
   the wrapper for the former is currently buggy (there is no easy way
   to detect setting changes) */
#define HEXBED_USE_NATIVE_PREFS 0

#endif /* HEXBED_COMMON_FLAGS_HH */
