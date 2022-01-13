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
// file/cisearch.cc -- impl for case insensitive search

#include "file/cisearch.hh"

#include <sstream>

#include "common/caseconv.hh"
#include "common/logger.hh"
#include "common/memory.hh"
#include "file/document.hh"
#include "file/search.hh"

namespace hexbed {

CaseInsensitivePattern::CaseInsensitivePattern()
    : encoding(TextEncoding::SBCS),
      sbcs(getSbcsByName("ascii")),
      pattern(),
      headLowerLen(0),
      headUpperLen(0) {}

CaseInsensitivePattern::CaseInsensitivePattern(const std::string& encname,
                                               const std::wstring& text_) {
    std::u32string text = wstringToU32string(text_);
    if (encname == "m_utf8") {
        encoding = TextEncoding::UTF8;
    } else if (encname == "m_utf16le") {
        encoding = TextEncoding::UTF16LE;
    } else if (encname == "m_utf16be") {
        encoding = TextEncoding::UTF16BE;
    } else if (encname == "m_utf32le") {
        encoding = TextEncoding::UTF32LE;
    } else if (encname == "m_utf32be") {
        encoding = TextEncoding::UTF32BE;
    } else {
        encoding = TextEncoding::SBCS;
        sbcs = getSbcsByName(encname);
    }
    pattern = textCaseFold(text);
    headUpperLen = encodeCharMbcsOrSbcs(encoding, sbcs, textCaseUpper(text)[0],
                                        sizeof(headUpper), headUpper);
    headLowerLen = encodeCharMbcsOrSbcs(encoding, sbcs, textCaseLower(text)[0],
                                        sizeof(headLower), headLower);
}

static std::u32string readU32String(const HexBedDocument& document,
                                    bufsize offset, bufsize wantedLength,
                                    bufsize& readBytes,
                                    CaseInsensitivePattern& pattern) {
    byte buf[512];
    bufsize r;
    u32ostringstream us;
    bufsize readchars = 0;
    bufsize leftover = 0;
    bufsize total = 0;
    while ((r = document.read(offset,
                              bytespan{buf + leftover, buf + sizeof(buf)}))) {
        DecodeStatus status =
            decodeStringMbcsOrSbcs(pattern.encoding, pattern.sbcs, us,
                                   const_bytespan{buf, buf + r + leftover});
        readchars += status.charCount;
        leftover = r - status.readCount;
        total += status.readCount;
        offset += status.readCount;
        if (!status.ok) break;
        if (readchars >= wantedLength) {
            std::u32string folded = textCaseFold(us.str());
            if (folded.length() >= wantedLength) {
                readBytes = total;
                return folded;
            }
        }
        if (leftover) memMove(buf, buf + status.readCount, leftover);
    }
    readBytes = total;
    return textCaseFold(us.str());
}

static bool equalsCaseInsensitive(const HexBedDocument& document,
                                  bufsize offset,
                                  CaseInsensitivePattern& pattern,
                                  bufsize& length) {
    byte tmp[MBCS_CHAR_MAX];
    std::size_t expected = pattern.pattern.length();
    std::u32string compareString =
        readU32String(document, offset, expected + 1, length, pattern);
    if (compareString.size() < expected) return false;
    if (std::u32string_view(compareString.data(), expected) !=
        std::u32string_view(pattern.pattern.data(), expected))
        return false;
    bufsize leftover = 0;
    for (std::size_t i = expected; i < compareString.size(); ++i)
        leftover += encodeCharMbcsOrSbcs(pattern.encoding, pattern.sbcs,
                                         compareString[i], sizeof(tmp), tmp);
    length -= leftover;
    return true;
}

SearchResult searchForwardCaseless(HexBedTask& task,
                                   const HexBedDocument& document,
                                   bufsize start, bufsize end,
                                   CaseInsensitivePattern& pattern) {
    const_bytespan udata{pattern.headUpper, pattern.headUpperLen};
    const_bytespan ldata{pattern.headLower, pattern.headLowerLen};
    bufsize r, rr, o = start, z = std::max(udata.size(), ldata.size()), c, hc;
    if (start >= end) return SearchResult{};
    if (!z) return SearchResult{SearchResultType::Full, start};
    bufsize mins = getMinimalSearchBufferSize(z),
            prefs = getPreferredSearchBufferSize(z);
    byte* bp = new (std::nothrow) byte[(c = prefs)];
    if (!bp) bp = new byte[(c = mins)];
    std::unique_ptr<byte[]> buffer(bp);
    hc = c >> 1;
    byte* flippers[2];
    flippers[0] = bp;
    flippers[1] = bp + hc;
    byte* flip = flippers[0];
    int flipindex = 1;

    SearchResult2 pres{};
    while ((rr = std::min(end - o, hc)),
           (r = document.read(o, bytespan(flip, rr)))) {
        if (task.isCancelled()) break;
        if (pres.type == SearchResultType::Partial) {
            pres = searchFullForward2(hc, flippers[flipindex], r, flip,
                                      pattern.headLowerLen, pattern.headLower,
                                      pattern.headUpperLen, pattern.headUpper,
                                      pres.offset, pres.secondary);
            if (pres) {
                bufsize oo = o + pres.offset, sl;
                if (equalsCaseInsensitive(document, oo, pattern, sl))
                    return SearchResult{SearchResultType::Full, oo, sl};
            }
        }
        pres = searchPartialForward2(r, flip, pattern.headLowerLen,
                                     pattern.headLower, pattern.headUpperLen,
                                     pattern.headUpper, r == hc);
        if (pres.type == SearchResultType::Full) {
            bufsize oo = o + pres.offset, sl;
            if (equalsCaseInsensitive(document, oo, pattern, sl))
                return SearchResult{SearchResultType::Full, oo, sl};
        }
        if (pres.type == SearchResultType::Partial) {
            flip = flippers[flipindex];
            flipindex = flipindex ^ 1;
        }
        if (r < rr) break;
        o += r;
    }
    return SearchResult{};
}

SearchResult searchBackwardCaseless(HexBedTask& task,
                                    const HexBedDocument& document,
                                    bufsize start, bufsize end,
                                    CaseInsensitivePattern& pattern) {
    const_bytespan udata{pattern.headUpper, pattern.headUpperLen};
    const_bytespan ldata{pattern.headLower, pattern.headLowerLen};
    bufsize r, rr, o = end, z = std::max(udata.size(), ldata.size()), c, hc;
    if (start >= end) return SearchResult{};
    if (!z) return SearchResult{SearchResultType::Full, end};
    bufsize mins = getMinimalSearchBufferSize(z),
            prefs = getPreferredSearchBufferSize(z);
    byte* bp = new (std::nothrow) byte[(c = prefs)];
    if (!bp) bp = new byte[(c = mins)];
    std::unique_ptr<byte[]> buffer(bp);
    hc = c >> 1;
    byte* flippers[2];
    flippers[0] = bp;
    flippers[1] = bp + hc;
    byte* flip = flippers[0];
    int flipindex = 1;

    SearchResult2 pres{};
    while ((rr = std::min(o - start, hc)) &&
           rr == (r = document.read(o - rr, bytespan(flip, rr)))) {
        if (task.isCancelled()) break;
        o -= r;
        if (pres.type == SearchResultType::Partial) {
            pres = searchFullBackward2(hc, flippers[flipindex], r, flip,
                                       pattern.headLowerLen, pattern.headLower,
                                       pattern.headUpperLen, pattern.headUpper,
                                       pres.offset, pres.secondary);
            if (pres) {
                bufsize oo = o + pres.offset - z + 1, sl;
                if (equalsCaseInsensitive(document, oo, pattern, sl))
                    return SearchResult{SearchResultType::Full, oo, sl};
            }
        }
        pres = searchPartialBackward2(r, flip, pattern.headLowerLen,
                                      pattern.headLower, pattern.headUpperLen,
                                      pattern.headUpper, o > 0);
        if (pres.type == SearchResultType::Full) {
            bufsize oo = o + pres.offset - z + 1, sl;
            if (equalsCaseInsensitive(document, oo, pattern, sl))
                return SearchResult{SearchResultType::Full, oo, sl};
        }
        if (pres.type == SearchResultType::Partial) {
            flip = flippers[flipindex];
            flipindex = flipindex ^ 1;
        }
    }
    return SearchResult{};
}

};  // namespace hexbed
