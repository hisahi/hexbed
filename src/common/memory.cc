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
// common/memory.cc -- impl for memory utilities

#include "common/memory.hh"

#include <algorithm>
#include <cstring>
#include <iterator>

namespace hexbed {

bufsize memCopy(byte* edi, const byte* esi, bufsize ecx) {
    std::copy(esi, esi + ecx, edi);
    return ecx;
}

bufsize memCopyBack(byte* edi, const byte* esi, bufsize ecx) {
    std::copy_backward(esi, esi + ecx, edi + ecx);
    return ecx;
}

bufsize memMove(byte* edi, const byte* esi, bufsize ecx) {
    std::memmove(edi, esi, ecx);
    return ecx;
}

bufsize memFill(byte* edi, byte al, bufsize ecx) {
    std::memset(edi, al, ecx);
    return ecx;
}

static bufsize memFillRepeatNaive(byte* edi, bufsize ebx, const byte* esi,
                                  bufsize ecx) {
    bufsize ecx_orig = ecx;
    while (ecx >= ebx) edi += memCopy(edi, esi, ebx), ecx -= ebx;
    while (ecx) edi += memCopy(edi, esi, ecx);
    return ecx_orig;
}

static bufsize memFillRepeatLog(byte* edi, bufsize ebx, const byte* esi,
                                bufsize ecx) {
    // ecx >> (much greater than) ebx
    bufsize edx = ecx;
    byte* ebp = edi;
    edi += memCopy(edi, esi, ebx);
    ecx -= ebx;
    while (ecx >= ebx) {
        edi += memCopy(edi, ebp, ebx);
        ecx -= ebx;
        ebx <<= 1;
    }
    memCopy(edi, ebp, ecx);
    return edx;
}

bufsize memFillRepeat(byte* edi, bufsize ebx, const byte* esi, bufsize ecx) {
    if (ebx <= 1) return memFill(edi, ebx ? *esi : 0, ecx);
    if (ecx <= ebx) return memCopy(edi, esi, ecx);
    if (ecx < 256 || ebx >= 32 || (ecx >> 4) < ebx)
        return memFillRepeatNaive(edi, ebx, esi, ecx);
    return memFillRepeatLog(edi, ebx, esi, ecx);
}

bufsize memReverse(byte* edi, bufsize ecx) {
    std::reverse(edi, edi + ecx);
    return ecx;
}

const byte* memFindFirst(const byte* start, const byte* end, byte c) {
    const byte* p = std::find(start, end, c);
    return p == end ? nullptr : p;
}

const byte* memFindLast(const byte* start, const byte* end, byte c) {
    using ptr = std::reverse_iterator<const byte*>;
    ptr p = std::find(ptr(end), ptr(start), c);
    return p.base() == start ? nullptr : p.base();
}

bool memEqual(const byte* a, const byte* b, bufsize n) {
    return !std::memcmp(a, b, n);
}

};  // namespace hexbed
