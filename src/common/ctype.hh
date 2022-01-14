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
// common/ctype.hh -- ctype overloads for strings

#ifndef HEXBED_COMMON_CTYPE_HH
#define HEXBED_COMMON_CTYPE_HH

#include <cctype>
#include <cwctype>

namespace hexbed {

int c_isalnum(char c) { return std::isalnum(c); }
int c_isalpha(char c) { return std::isalpha(c); }
int c_isblank(char c) { return std::isblank(c); }
int c_iscntrl(char c) { return std::iscntrl(c); }
int c_isdigit(char c) { return std::isdigit(c); }
int c_isgraph(char c) { return std::isgraph(c); }
int c_islower(char c) { return std::islower(c); }
int c_isprint(char c) { return std::isprint(c); }
int c_ispunct(char c) { return std::ispunct(c); }
int c_isspace(char c) { return std::isspace(c); }
int c_isupper(char c) { return std::isupper(c); }
int c_isxdigit(char c) { return std::isxdigit(c); }
int c_tolower(char c) { return std::tolower(c); }
int c_toupper(char c) { return std::toupper(c); }

int c_isalnum(wchar_t c) { return std::iswalnum(c); }
int c_isalpha(wchar_t c) { return std::iswalpha(c); }
int c_isblank(wchar_t c) { return std::iswblank(c); }
int c_iscntrl(wchar_t c) { return std::iswcntrl(c); }
int c_isdigit(wchar_t c) { return std::iswdigit(c); }
int c_isgraph(wchar_t c) { return std::iswgraph(c); }
int c_islower(wchar_t c) { return std::iswlower(c); }
int c_isprint(wchar_t c) { return std::iswprint(c); }
int c_ispunct(wchar_t c) { return std::iswpunct(c); }
int c_isspace(wchar_t c) { return std::iswspace(c); }
int c_isupper(wchar_t c) { return std::iswupper(c); }
int c_isxdigit(wchar_t c) { return std::iswxdigit(c); }
int c_tolower(wchar_t c) { return std::towlower(c); }
int c_toupper(wchar_t c) { return std::towupper(c); }

};  // namespace hexbed

#endif /* HEXBED_COMMON_CTYPE_HH */
