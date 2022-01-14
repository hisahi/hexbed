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
// ui/config.hh -- header for UI-specific config helpers

#ifndef HEXBED_UI_CONFIG_HH
#define HEXBED_UI_CONFIG_HH

#include <wx/font.h>

#include "app/config.hh"

namespace hexbed {

namespace ui {

constexpr int MINIMUM_HEX_FONTSIZE = 3;
constexpr int MAXIMUM_HEX_FONTSIZE = 144;

wxFont getHexFontOrDefault(const string& s);
string hexFontToString(const wxFont& font);

wxFont configFont();
void configFont(const wxFont& font);

};  // namespace ui

};  // namespace hexbed

#endif /* HEXBED_UI_CONFIG_HH */
