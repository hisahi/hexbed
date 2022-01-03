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
// file/search.cc -- impl for byte string searching with two buffers

#include "file/search.hh"

#include "common/logger.hh"
#include "common/memory.hh"

namespace hexbed {

SearchResult searchPartialForward(bufsize slen, const byte* sdata, bufsize nlen,
                                  const byte* ndata, bool allowPartial) {
    const byte* send = sdata + slen;
    const byte* schr = sdata;
    SearchResultType mtype = SearchResultType::Full;
    byte header = ndata[0];
    ++ndata;
    --nlen;
    while ((schr = memFindFirst(schr, send, header))) {
        bufsize off = static_cast<bufsize>(schr - sdata);
        ++schr;
        bufsize rem = send - schr;
        if (rem < nlen) {
            if (!allowPartial) break;
            mtype = SearchResultType::Partial;
            nlen = rem;
        }
        if (memEqual(schr, ndata, nlen)) return SearchResult{mtype, off};
    }
    return SearchResult{};
}

static bool tryFullMatchForward(bufsize alen, const byte* adata, bufsize blen,
                                const byte* bdata, bufsize nlen,
                                const byte* ndata, bufsize off) {
    bufsize lo = alen - off;
    if (nlen < lo) return memEqual(adata + off, ndata, nlen);
    return memEqual(adata + off, ndata, lo) &&
           memEqual(bdata, ndata + lo, nlen - lo);
}

SearchResult searchFullForward(bufsize alen, const byte* adata, bufsize blen,
                               const byte* bdata, bufsize nlen,
                               const byte* ndata, bufsize off) {
    const byte* aend = adata + alen;
    const byte* ap = adata + off;
    byte header = ndata[0];
    HEXBED_ASSERT(*ap == header);
    do {
        off = ap - adata;
        if (tryFullMatchForward(alen, adata, blen, bdata, nlen, ndata, off))
            return SearchResult{SearchResultType::Full, off};
        ap = memFindFirst(ap + 1, aend, header);
    } while (ap);
    return SearchResult{};
}

SearchResult searchPartialBackward(bufsize slen, const byte* sdata,
                                   bufsize nlen, const byte* ndata,
                                   bool allowPartial) {
    const byte* send = sdata + slen;
    const byte* schr = send;
    SearchResultType mtype = SearchResultType::Full;
    byte footer = ndata[--nlen];
    while ((schr = memFindLast(sdata, schr, footer))) {
        bufsize rem = schr - sdata;
        if (rem < nlen) {
            if (!allowPartial) break;
            mtype = SearchResultType::Partial;
            ndata += nlen - rem;
            nlen = rem;
        }
        if (memEqual(schr - nlen, ndata, nlen)) return SearchResult{mtype, rem};
    }
    return SearchResult{};
}

static bool tryFullMatchBackward(bufsize alen, const byte* adata, bufsize blen,
                                 const byte* bdata, bufsize nlen,
                                 const byte* ndata, bufsize off) {
    if (off >= nlen) return memEqual(adata + off - nlen, ndata, nlen);
    bufsize lo = nlen - off;
    return memEqual(adata, ndata + lo, off) &&
           memEqual(bdata + blen - lo, ndata, lo);
}

SearchResult searchFullBackward(bufsize alen, const byte* adata, bufsize blen,
                                const byte* bdata, bufsize nlen,
                                const byte* ndata, bufsize off) {
    const byte* ap = adata + off;
    byte footer = ndata[--nlen];
    HEXBED_ASSERT(*ap == footer);
    do {
        off = ap - adata;
        if (tryFullMatchBackward(alen, adata, blen, bdata, nlen, ndata, off))
            return SearchResult{SearchResultType::Full, off};
        ap = memFindLast(adata, ap, footer);
    } while (ap);
    return SearchResult{};
}

};  // namespace hexbed
