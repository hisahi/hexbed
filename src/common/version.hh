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
// common/version.hh -- header for the application version

#ifndef HEXBED_COMMON_VERSION_HH
#define HEXBED_COMMON_VERSION_HH

#define HEXBED_VER_MAJOR 0
#define HEXBED_VER_MINOR 7
#define HEXBED_VER_PATCH 0

#define HEXBED_STRINGIFY_RAW(x) #x
#define HEXBED_STRINGIFY(x) HEXBED_STRINGIFY_RAW(x)

#define HEXBED_VER_STRING                                        \
    HEXBED_STRINGIFY(HEXBED_VER_MAJOR)                           \
    "." HEXBED_STRINGIFY(HEXBED_VER_MINOR) "." HEXBED_STRINGIFY( \
        HEXBED_VER_PATCH)

#endif /* HEXBED_COMMON_VERSION_HH */
