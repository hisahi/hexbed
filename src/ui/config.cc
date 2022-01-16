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
#include "common/specs.hh"
#include "ui/string.hh"

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

wxFont getHexFontOrDefault(const string& s) {
    if (s.empty()) return getDefaultHexFont();
    wxFont fnt(s);
    if (!fnt.IsOk() || !inNumericRange(MINIMUM_HEX_FONTSIZE, fnt.GetPointSize(),
                                       MAXIMUM_HEX_FONTSIZE))
        fnt = getDefaultHexFont();
    HEXBED_ASSERT(fnt.IsOk());
    return fnt;
}

string hexFontToString(const wxFont& font) {
    return stringFromWx(font.GetNativeFontInfoDesc());
}

wxFont configFont() { return getHexFontOrDefault(config().font); }

void configFont(const wxFont& font) {
    currentConfig.values().font = hexFontToString(font);
}

unsigned configUtfGroupSizeL2() {
    switch (config().utfMode) {
    default:
        HEXBED_UNREACHABLE();
    case 0:
        return 0;
    case 1:
    case 2:
        return 1;
    case 3:
    case 4:
        return 2;
    }
}

unsigned configUtfGroupSize() { return 1U << configUtfGroupSizeL2(); }

};  // namespace ui

};  // namespace hexbed
