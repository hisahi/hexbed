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
// common/config.cc -- impl for application configuration

#include "common/config.hh"

#include <bit>
#include <limits>

#include "common/charconv.hh"
#include "common/logger.hh"

namespace hexbed {

void Configuration::loadValues() {
    values_.language = loadString("language", "");
    values_.backgroundColor = loadColor("backgroundColor", 0x000000L);
    values_.offsetColor = loadColor("offsetColor", 0xAAAA66L);
    values_.hexColorOdd = loadColor("hexColorOdd", 0xFFFFFFL);
    values_.hexColorEven = loadColor("hexColorEven", 0xCCCCCCL);
    values_.textColor = loadColor("textColor", 0xEEEEEEL);
    values_.textNonprintableColor =
        loadColor("textNonprintableColor", 0x999999L);
    values_.alignColor = loadColor("alignColor", 0x777777L);
    values_.selectBackgroundColor =
        loadColor("selectBackgroundColor", 0xFFFFFFL);
    values_.selectForegroundColor =
        loadColor("selectForegroundColor", 0x000000L);
    values_.selectAltBackgroundColor =
        loadColor("selectAltBackgroundColor", 0x555555L);
    values_.selectAltForegroundColor =
        loadColor("selectAltForegroundColor", 0xEEEEEEL);
    values_.hexColumns = loadIntRange("hexColumns", 16, 1, MAX_COLUMNS);
    values_.uppercase = loadBool("uppercase", true);
    values_.undoHistoryMaximum = loadIntRange("undoHistoryMaximum", 500, 0,
                                              std::numeric_limits<long>::max());
    values_.groupSize = static_cast<long>(std::bit_floor<unsigned>(
        static_cast<unsigned>(loadIntRange("groupSize", 1, 1, 16))));
    values_.offsetRadix =
        static_cast<unsigned>(loadIntSet("offsetRadix", 16, {8, 10, 16}));
    values_.autoFit = loadBool("autoFit", false);
    values_.charset = loadString("charset", "ascii");
    values_.font = loadString("font", "");
    values_.showColumnTypes = loadIntRange("showColumnTypes", 3, 1, 3);
    values_.backupFiles = loadBool("backupFiles", true);
    values_.utfMode = loadIntRange("utfMode", 0, 4, 0);
}

void Configuration::saveValues() {
    saveString("language", values_.language);
    saveInt("backgroundColor", values_.backgroundColor);
    saveInt("offsetColor", values_.offsetColor);
    saveInt("hexColorOdd", values_.hexColorOdd);
    saveInt("hexColorEven", values_.hexColorEven);
    saveInt("textColor", values_.textColor);
    saveInt("textNonprintableColor", values_.textNonprintableColor);
    saveInt("alignColor", values_.alignColor);
    saveInt("selectBackgroundColor", values_.selectBackgroundColor);
    saveInt("selectForegroundColor", values_.selectForegroundColor);
    saveInt("selectAltBackgroundColor", values_.selectAltBackgroundColor);
    saveInt("selectAltForegroundColor", values_.selectAltForegroundColor);
    saveInt("hexColumns", values_.hexColumns);
    saveBool("uppercase", values_.uppercase);
    saveInt("undoHistoryMaximum", values_.undoHistoryMaximum);
    saveInt("groupSize", values_.groupSize);
    saveInt("offsetRadix", values_.offsetRadix);
    saveBool("autoFit", values_.autoFit);
    saveString("charset", values_.charset);
    saveString("font", values_.font);
    saveInt("showColumnTypes", values_.showColumnTypes);
    saveBool("backupFiles", values_.backupFiles);
    saveInt("utfMode", values_.utfMode);
}

long Configuration::loadColor(const string& key, long def) {
    return loadIntRange(key, def, 0x000000L, 0xFFFFFFL);
}

long Configuration::loadIntRange(const string& key, long def, long min,
                                 long max) {
    long r = loadInt(key, def);
    if (r != def && (r < min || r > max)) r = def;
    return r;
}

long Configuration::loadIntSet(const string& key, long def,
                               std::initializer_list<long> pass) {
    long r = loadInt(key, def);
    if (r == def) return r;
    for (long l : pass)
        if (l == r) return r;
    return def;
}

}  // namespace hexbed
