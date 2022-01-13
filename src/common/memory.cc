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

bufsize memCopy(byte* edi, const byte* esi, bufsize ecx) noexcept {
    std::copy(esi, esi + ecx, edi);
    return ecx;
}

bufsize memCopyBack(byte* edi, const byte* esi, bufsize ecx) noexcept {
    std::copy_backward(esi, esi + ecx, edi + ecx);
    return ecx;
}

bufsize memMove(byte* edi, const byte* esi, bufsize ecx) noexcept {
    std::memmove(edi, esi, ecx);
    return ecx;
}

bufsize memFill(byte* edi, byte al, bufsize ecx) noexcept {
    std::memset(edi, al, ecx);
    return ecx;
}

static bufsize memFillRepeatNaive(byte* edi, bufsize ebx, const byte* esi,
                                  bufsize ecx) noexcept {
    bufsize ecx_orig = ecx;
    while (ecx >= ebx) edi += memCopy(edi, esi, ebx), ecx -= ebx;
    if (ecx) edi += memCopy(edi, esi, ecx);
    return ecx_orig;
}

static bufsize memFillRepeatLog(byte* edi, bufsize ebx, const byte* esi,
                                bufsize ecx) noexcept {
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

bufsize memFillRepeat(byte* edi, bufsize ebx, const byte* esi,
                      bufsize ecx) noexcept {
    if (ebx <= 1) return memFill(edi, ebx ? *esi : 0, ecx);
    if (ecx <= ebx) return memCopy(edi, esi, ecx);
    if (ecx < 256 || ebx >= 32 || (ecx >> 4) < ebx)
        return memFillRepeatNaive(edi, ebx, esi, ecx);
    return memFillRepeatLog(edi, ebx, esi, ecx);
}

bufsize memReverse(byte* edi, bufsize ecx) noexcept {
    std::reverse(edi, edi + ecx);
    return ecx;
}

bool memEqual(const byte* a, const byte* b, bufsize n) noexcept {
    return !std::memcmp(a, b, n);
}

// first match in [start, end) or null
const byte* memFindFirst(const byte* start, const byte* end, byte c) noexcept {
    return reinterpret_cast<const byte*>(std::memchr(start, c, end - start));
}

static const void* memrchr(const void* ptr, int ch,
                           std::size_t count) noexcept {
    const unsigned char* esi = reinterpret_cast<const unsigned char*>(ptr);
    unsigned char al = static_cast<unsigned char>(ch);
    esi += count;
    while (count--)
        if (*--esi == al) return esi;
    return nullptr;
}

// final match in [start, end) or null
const byte* memFindLast(const byte* start, const byte* end, byte c) noexcept {
    return reinterpret_cast<const byte*>(memrchr(start, c, end - start));
}

static constexpr bufsize alternateSize = 1024;

const byte* memFindFirst2(const byte* start, const byte* end, byte c1,
                          byte c2) noexcept {
    bufsize z = end - start;
    const byte* p = start;
    while (z > alternateSize) {
        const byte* pt = p + alternateSize;
        const byte* p1 = memFindFirst(p, pt, c1);
        const byte* p2 = memFindFirst(p, p1 ? p1 : pt, c2);
        if (p1) {
            return p2 ? std::min(p1, p2) : p1;
        } else if (p2)
            return p2;
        p = pt;
        z -= alternateSize;
    }
    const byte* p1 = memFindFirst(p, end, c1);
    const byte* p2 = memFindFirst(p, p1 ? p1 : end, c2);
    return p1 ? (p2 ? std::min(p1, p2) : p1) : p2;
}

const byte* memFindLast2(const byte* start, const byte* end, byte c1,
                         byte c2) noexcept {
    bufsize z = end - start;
    const byte* p = end;
    while (z > alternateSize) {
        const byte* pt = p - alternateSize;
        const byte* p1 = memFindLast(pt, p, c1);
        const byte* p2 = memFindLast(p1 ? p1 : pt, p, c2);
        if (p1) {
            return p2 ? std::max(p1, p2) : p1;
        } else if (p2)
            return p2;
        p = pt;
        z -= alternateSize;
    }
    const byte* p1 = memFindLast(start, p, c1);
    const byte* p2 = memFindLast(p1 ? p1 : start, p, c2);
    return p1 ? (p2 ? std::max(p1, p2) : p1) : p2;
}

};  // namespace hexbed
