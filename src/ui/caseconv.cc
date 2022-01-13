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
// ui/caseconv.cc -- wxWidgets impl for text case conversions

#include "common/caseconv.hh"

#include <wx/string.h>

#include "common/charconv.hh"

namespace hexbed {

std::u32string textCaseFold(const std::u32string& text) {
    // wxWidgets has no separate case fold function so use upper
    return textCaseUpper(text);
}

std::u32string textCaseUpper(const std::u32string& text) {
    return wstringToU32string(
        wxString(u32stringToWstring(text)).MakeUpper().ToStdWstring());
}

std::u32string textCaseLower(const std::u32string& text) {
    return wstringToU32string(
        wxString(u32stringToWstring(text)).MakeLower().ToStdWstring());
}

};  // namespace hexbed
