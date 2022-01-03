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
// common/memory.hh -- header for memory utilities

#ifndef HEXBED_COMMON_MEMORY_HH
#define HEXBED_COMMON_MEMORY_HH

#include "common/types.hh"

namespace hexbed {

bufsize memCopy(byte* edi, const byte* esi, bufsize ecx);
bufsize memCopyBack(byte* edi, const byte* esi, bufsize ecx);
bufsize memMove(byte* edi, const byte* esi, bufsize ecx);
bufsize memFill(byte* edi, byte al, bufsize ecx);
const byte* memFindFirst(const byte* start, const byte* end, byte c);
const byte* memFindLast(const byte* start, const byte* end, byte c);
bool memEqual(const byte* a, const byte* b, bufsize n);

};  // namespace hexbed

#endif /* HEXBED_COMMON_MEMORY_HH */
