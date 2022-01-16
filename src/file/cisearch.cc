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

#include "app/encoding.hh"
#include "common/buffer.hh"
#include "common/caseconv.hh"
#include "common/logger.hh"
#include "common/memory.hh"
#include "file/document.hh"
#include "file/search.hh"

namespace hexbed {

CaseInsensitivePattern::CaseInsensitivePattern()
    : encoding(getCharacterEncodingByName(STRING(""))),
      pattern(),
      headLowerLen(0),
      headUpperLen(0) {}

static std::size_t encodeOneChar(const CharacterEncoding& enc, char32_t c,
                                 std::size_t n, byte* b) {
    CharEncodeStatus status =
        enc.encode(charEncodeFromArray(1, &c), charEncodeToArray(n, b));
    return status.ok ? status.wroteBytes : 0;
}

CaseInsensitivePattern::CaseInsensitivePattern(const string& encname,
                                               const std::wstring& text_)
    : encoding(getCharacterEncodingByName(encname)) {
    std::u32string text = wstringToU32string(text_);
    pattern = textCaseFold(text);
    headUpperLen = encodeOneChar(encoding, textCaseUpper(text)[0],
                                 sizeof(headUpper), headUpper);
    headLowerLen = encodeOneChar(encoding, textCaseLower(text)[0],
                                 sizeof(headLower), headLower);
}

static std::u32string readU32String(const HexBedDocument& document,
                                    bufsize offset, bufsize wantedLength,
                                    bufsize& readBytes,
                                    CaseInsensitivePattern& pattern) {
    byte buf[BUFFER_SIZE];
    bufsize r;
    u32ostringstream us;
    bufsize readchars = 0;
    bufsize leftover = 0;
    bufsize total = 0;
    while ((r = document.read(offset,
                              bytespan{buf + leftover, buf + sizeof(buf)}))) {
        CharDecodeStatus status = pattern.encoding.decode(
            charDecodeFromArray(r + leftover, buf), charDecodeToStream(us));
        readchars += status.wroteChars;
        leftover = r - status.readBytes;
        total += status.readBytes;
        offset += status.readBytes;
        if (!status.ok) break;
        if (readchars >= wantedLength) {
            std::u32string folded = textCaseFold(us.str());
            if (folded.length() >= wantedLength) {
                readBytes = total;
                return folded;
            }
        }
        if (leftover) memMove(buf, buf + status.readBytes, leftover);
    }
    readBytes = total;
    return textCaseFold(us.str());
}

static bool equalsCaseInsensitive(const HexBedDocument& document,
                                  bufsize offset,
                                  CaseInsensitivePattern& pattern,
                                  bufsize& length) {
    std::size_t expected = pattern.pattern.length();
    std::u32string compareString =
        readU32String(document, offset, expected + 1, length, pattern);
    if (compareString.size() < expected) return false;
    if (std::u32string_view(compareString.data(), expected) !=
        std::u32string_view(pattern.pattern.data(), expected))
        return false;
    if (expected < compareString.size())
        length -=
            pattern.encoding
                .encode(charEncodeFromArray(compareString.size() - expected,
                                            compareString.data()),
                        charEncodeToNull())
                .wroteBytes;
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
