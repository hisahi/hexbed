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
        if (memEqual(schr, ndata, nlen)) return SearchResult{mtype, off, nlen};
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
            return SearchResult{SearchResultType::Full, off, nlen};
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
        if (memEqual(schr - nlen, ndata, nlen))
            return SearchResult{mtype, rem, nlen};
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
            return SearchResult{SearchResultType::Full, off, nlen};
        ap = memFindLast(adata, ap, footer);
    } while (ap);
    return SearchResult{};
}

static SearchResult2 s1to2(const SearchResult& result, bool secondary) {
    return SearchResult2{result.type, result.offset, result.length, secondary};
}

SearchResult2 searchPartialForward2(bufsize slen, const byte* sdata,
                                    bufsize nlen, const byte* ndata,
                                    bufsize mlen, const byte* mdata,
                                    bool allowPartial) {
    if (!nlen)
        return s1to2(
            searchPartialForward(slen, sdata, mlen, mdata, allowPartial), true);
    if (!mlen)
        return s1to2(
            searchPartialForward(slen, sdata, nlen, ndata, allowPartial),
            false);
    const byte* send = sdata + slen;
    const byte* schr = sdata;
    SearchResultType mtype = SearchResultType::Full;
    byte header1 = ndata[0];
    ++ndata;
    --nlen;
    byte header2 = mdata[0];
    ++mdata;
    --mlen;
    while ((schr = memFindFirst2(schr, send, header1, header2))) {
        bufsize off = static_cast<bufsize>(schr - sdata);
        ++schr;
        bufsize rem = send - schr;
        bool secondary = *schr == header2;
        bufsize qlen = secondary ? mlen : nlen;
        const byte* qdata = secondary ? mdata : ndata;
        if (rem < qlen) {
            if (!allowPartial) break;
            mtype = SearchResultType::Partial;
            mlen = std::min(mlen, rem);
            nlen = std::min(nlen, rem);
        }
        if (memEqual(schr, qdata, qlen))
            return SearchResult2{mtype, off, qlen, secondary};
    }
    return SearchResult2{};
}

SearchResult2 searchFullForward2(bufsize alen, const byte* adata, bufsize blen,
                                 const byte* bdata, bufsize nlen,
                                 const byte* ndata, bufsize mlen,
                                 const byte* mdata, bufsize off,
                                 bool secondary) {
    if (!nlen)
        return s1to2(
            searchFullForward(alen, adata, blen, bdata, mlen, mdata, off),
            true);
    if (!mlen)
        return s1to2(
            searchFullForward(alen, adata, blen, bdata, nlen, ndata, off),
            false);
    const byte* aend = adata + alen;
    const byte* ap = adata + off;
    byte header1 = ndata[0];
    byte header2 = mdata[0];
    HEXBED_ASSERT(*ap == (secondary ? header2 : header1));
    do {
        bufsize qlen = secondary ? mlen : nlen;
        const byte* qdata = secondary ? mdata : ndata;
        off = ap - adata;
        if (tryFullMatchForward(alen, adata, blen, bdata, qlen, qdata, off))
            return SearchResult2{SearchResultType::Full, off, qlen, secondary};
        ap = memFindFirst2(ap + 1, aend, header1, header2);
        if (ap) secondary = *ap == header2;
    } while (ap);
    return SearchResult2{};
}

SearchResult2 searchPartialBackward2(bufsize slen, const byte* sdata,
                                     bufsize nlen, const byte* ndata,
                                     bufsize mlen, const byte* mdata,
                                     bool allowPartial) {
    if (!nlen)
        return s1to2(
            searchPartialForward(slen, sdata, mlen, mdata, allowPartial), true);
    if (!mlen)
        return s1to2(
            searchPartialForward(slen, sdata, nlen, ndata, allowPartial),
            false);
    const byte* send = sdata + slen;
    const byte* schr = send;
    SearchResultType mtype = SearchResultType::Full;
    byte footer1 = ndata[--nlen];
    byte footer2 = mdata[--mlen];
    while ((schr = memFindLast2(sdata, schr, footer1, footer2))) {
        bufsize rem = schr - sdata;
        bool secondary = *schr == footer2;
        bufsize qlen = secondary ? mlen : nlen;
        const byte* qdata = secondary ? mdata : ndata;
        if (rem < qlen) {
            if (!allowPartial) break;
            mtype = SearchResultType::Partial;
            if (nlen > rem) {
                ndata += nlen - rem;
                nlen = rem;
            }
            if (mlen > rem) {
                mdata += mlen - rem;
                mlen = rem;
            }
        }
        if (memEqual(schr - qlen, qdata, qlen))
            return SearchResult2{mtype, rem, qlen, secondary};
    }
    return SearchResult2{};
}

SearchResult2 searchFullBackward2(bufsize alen, const byte* adata, bufsize blen,
                                  const byte* bdata, bufsize nlen,
                                  const byte* ndata, bufsize mlen,
                                  const byte* mdata, bufsize off,
                                  bool secondary) {
    if (!nlen)
        return s1to2(
            searchFullBackward(alen, adata, blen, bdata, mlen, mdata, off),
            true);
    if (!mlen)
        return s1to2(
            searchFullBackward(alen, adata, blen, bdata, nlen, ndata, off),
            false);
    const byte* ap = adata + off;
    byte footer1 = ndata[--nlen];
    byte footer2 = mdata[--mlen];
    HEXBED_ASSERT(*ap == (secondary ? footer2 : footer1));
    do {
        bufsize qlen = secondary ? mlen : nlen;
        const byte* qdata = secondary ? mdata : ndata;
        off = ap - adata;
        if (tryFullMatchBackward(alen, adata, blen, bdata, qlen, qdata, off))
            return SearchResult2{SearchResultType::Full, off, qlen, secondary};
        ap = memFindLast2(adata, ap, footer1, footer2);
        if (ap) secondary = *ap == footer2;
    } while (ap);
    return SearchResult2{};
}

bufsize getPreferredSearchBufferSize(bufsize s) {
    if (s >= (1 << 24)) return s << 1;
    if (s >= (1 << 20)) return s << 2;
    if (s >= (1 << 16)) return s << 3;
    if (s >= (1 << 12)) return s << 4;
    return (1 << 12) << 4;
}

bufsize getMinimalSearchBufferSize(bufsize s) { return s * 2; }

};  // namespace hexbed
