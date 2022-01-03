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
// ui/config.cc -- impl for UI-specific config helpers

#include "ui/config.hh"

#include "common/logger.hh"

namespace hexbed {

namespace ui {

template <typename T>
static bool inNumericRange(const T& min, const T& val, const T& max) {
    return min <= val && val <= max;
}

static wxFont getDefaultHexFont() {
    return wxFont(12, wxFONTFAMILY_TELETYPE, wxFONTSTYLE_NORMAL,
                  wxFONTWEIGHT_NORMAL);
}

wxFont getHexFontOrDefault(const std::string& s) {
    if (s.empty()) return getDefaultHexFont();
    wxFont fnt(s);
    if (!fnt.IsOk() || !inNumericRange(MINIMUM_HEX_FONTSIZE, fnt.GetPointSize(),
                                       MAXIMUM_HEX_FONTSIZE))
        fnt = getDefaultHexFont();
    HEXBED_ASSERT(fnt.IsOk());
    return fnt;
}

std::string hexFontToString(const wxFont& font) {
    return font.GetNativeFontInfoDesc().ToStdString();
}

wxFont configFont() { return getHexFontOrDefault(config().font); }

void configFont(const wxFont& font) {
    currentConfig.values().font = hexFontToString(font);
}

};  // namespace ui

};  // namespace hexbed
