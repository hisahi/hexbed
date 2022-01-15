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
// common/config.hh -- header for application configuration

#ifndef HEXBED_COMMON_CONFIG_HH
#define HEXBED_COMMON_CONFIG_HH

#include <initializer_list>

#include "common/types.hh"

namespace hexbed {

/* careful! increasing this limit breaks octal offsets because 64 - 1 = 63 is
   the largest number that can be represented in octal only with two digits,
   and HexBed currently assumes all column offsets to meet this requirement */
constexpr unsigned MAX_COLUMNS = 64;

struct ConfigurationValues {
    string language;
    long backgroundColor;
    long offsetColor;
    long hexColorOdd;
    long hexColorEven;
    long textColor;
    long textNonprintableColor;
    long alignColor;
    long selectBackgroundColor;
    long selectForegroundColor;
    long selectAltBackgroundColor;
    long selectAltForegroundColor;
    long hexColumns;
    bool uppercase;
    long undoHistoryMaximum;
    long groupSize;
    long offsetRadix;
    bool autoFit;
    string charset;
    string font;
    long showColumnTypes;
    bool backupFiles;
};

class Configuration {
  public:
    inline ConfigurationValues& values() { return values_; }

  protected:
    ConfigurationValues values_;

    void loadValues();
    void saveValues();

    long loadIntRange(const string& key, long def, long min, long max);
    long loadIntSet(const string& key, long def,
                    std::initializer_list<long> pass);
    long loadColor(const string& key, long def);

    virtual bool loadBool(const string& key, bool def) = 0;
    virtual long loadInt(const string& key, long def) = 0;
    virtual float loadFloat(const string& key, double def) = 0;
    virtual string loadString(const string& key, const string& def) = 0;

    virtual void saveBool(const string& key, bool value) = 0;
    virtual void saveInt(const string& key, long value) = 0;
    virtual void saveFloat(const string& key, double value) = 0;
    virtual void saveString(const string& key, const string& value) = 0;
};

};  // namespace hexbed

#endif /* HEXBED_COMMON_CONFIG_HH */
