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
// file/cisearch.hh -- header for case-insensitive search

#ifndef HEXBED_FILE_CISEARCH_HH
#define HEXBED_FILE_CISEARCH_HH

#include "common/charconv.hh"
#include "file/document-fwd.hh"
#include "file/search.hh"
#include "file/task.hh"

namespace hexbed {

struct CaseInsensitivePattern {
    CaseInsensitivePattern();
    CaseInsensitivePattern(const string& encoding, const std::wstring& text);

    TextEncoding encoding;
    SingleByteCharacterSet sbcs;
    std::u32string pattern;
    bufsize headLowerLen;
    byte headLower[MBCS_CHAR_MAX];
    bufsize headUpperLen;
    byte headUpper[MBCS_CHAR_MAX];
};

SearchResult searchForwardCaseless(HexBedTask& task,
                                   const HexBedDocument& document,
                                   bufsize start, bufsize end,
                                   CaseInsensitivePattern& pattern);
SearchResult searchBackwardCaseless(HexBedTask& task,
                                    const HexBedDocument& document,
                                    bufsize start, bufsize end,
                                    CaseInsensitivePattern& pattern);

};  // namespace hexbed

#endif /* HEXBED_FILE_SEARCH_HH */
