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

namespace hexbed {

int c_isalnum(char c);
int c_isalpha(char c);
int c_isblank(char c);
int c_iscntrl(char c);
int c_isdigit(char c);
int c_isgraph(char c);
int c_islower(char c);
int c_isprint(char c);
int c_ispunct(char c);
int c_isspace(char c);
int c_isupper(char c);
int c_isxdigit(char c);
int c_tolower(char c);
int c_toupper(char c);

int c_isalnum(wchar_t c);
int c_isalpha(wchar_t c);
int c_isblank(wchar_t c);
int c_iscntrl(wchar_t c);
int c_isdigit(wchar_t c);
int c_isgraph(wchar_t c);
int c_islower(wchar_t c);
int c_isprint(wchar_t c);
int c_ispunct(wchar_t c);
int c_isspace(wchar_t c);
int c_isupper(wchar_t c);
int c_isxdigit(wchar_t c);
int c_tolower(wchar_t c);
int c_toupper(wchar_t c);

};  // namespace hexbed

#endif /* HEXBED_COMMON_CTYPE_HH */
