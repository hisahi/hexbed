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
// file/search.hh -- header for byte string searching with two buffers

#ifndef HEXBED_FILE_SEARCH_HH
#define HEXBED_FILE_SEARCH_HH

#include "common/charconv.hh"
#include "common/types.hh"

namespace hexbed {

enum class SearchResultType { None, Full, Partial };

struct SearchResult {
    SearchResultType type{SearchResultType::None};
    bufoffset offset{0};
    bufoffset length{0};

    inline operator bool() const noexcept {
        return type != SearchResultType::None;
    }
};

struct SearchResult2 {
    SearchResultType type{SearchResultType::None};
    bufoffset offset{0};
    bufoffset length{0};
    bool secondary{false};

    inline operator bool() const noexcept {
        return type != SearchResultType::None;
    }
};

SearchResult searchPartialForward(bufsize slen, const byte* sdata, bufsize nlen,
                                  const byte* ndata, bool allowPartial);
SearchResult searchFullForward(bufsize alen, const byte* adata, bufsize blen,
                               const byte* bdata, bufsize nlen,
                               const byte* ndata, bufsize off);

SearchResult searchPartialBackward(bufsize slen, const byte* sdata,
                                   bufsize nlen, const byte* ndata,
                                   bool allowPartial);
SearchResult searchFullBackward(bufsize alen, const byte* adata, bufsize blen,
                                const byte* bdata, bufsize nlen,
                                const byte* ndata, bufsize off);

SearchResult2 searchPartialForward2(bufsize slen, const byte* sdata,
                                    bufsize nlen, const byte* ndata,
                                    bufsize mlen, const byte* mdata,
                                    bool allowPartial);
SearchResult2 searchFullForward2(bufsize alen, const byte* adata, bufsize blen,
                                 const byte* bdata, bufsize nlen,
                                 const byte* ndata, bufsize mlen,
                                 const byte* mdata, bufsize off,
                                 bool secondary);

SearchResult2 searchPartialBackward2(bufsize slen, const byte* sdata,
                                     bufsize nlen, const byte* ndata,
                                     bufsize mlen, const byte* mdata,
                                     bool allowPartial);
SearchResult2 searchFullBackward2(bufsize alen, const byte* adata, bufsize blen,
                                  const byte* bdata, bufsize nlen,
                                  const byte* ndata, bufsize mlen,
                                  const byte* mdata, bufsize off,
                                  bool secondary);

bufsize getMinimalSearchBufferSize(bufsize s);
bufsize getPreferredSearchBufferSize(bufsize s);

};  // namespace hexbed

#endif /* HEXBED_FILE_SEARCH_HH */
