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
// common/caseconv.cc -- base impl for text case conversions

#include "common/caseconv.hh"

// in the standard HexBed wxWidgets setup, this is not compiled in,
// because ui/caseconv.cc replaces it

#include <string>

namespace hexbed {

static char32_t charUpper(char32_t c) {
    if (U'a' <= c && c <= U'z') return c - U'a' + U'A';
    return c;
}

static char32_t charLower(char32_t c) {
    if (U'A' <= c && c <= U'Z') return c - U'A' + U'a';
    return c;
}

std::u32string textCaseFold(const std::u32string& text) {
    return textCaseUpper(text);
}

std::u32string textCaseUpper(const std::u32string& text) {
    std::size_t n = text.size();
    std::u32string result(n, U' ');
    char32_t* p = result.data();
    for (std::size_t i = 0; i < n; ++i) p[i] = charUpper(text[i]);
    return result;
}

std::u32string textCaseLower(const std::u32string& text) {
    std::size_t n = text.size();
    std::u32string result(n, U' ');
    char32_t* p = result.data();
    for (std::size_t i = 0; i < n; ++i) p[i] = charLower(text[i]);
    return result;
}

};  // namespace hexbed
