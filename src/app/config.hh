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
// app/config.hh -- used to include main configuration file

#ifndef HEXBED_APP_CONFIG_HH
#define HEXBED_APP_CONFIG_HH

#include <ostream>
#include <unordered_map>

#include "common/config.hh"
#include "common/types.hh"

namespace hexbed {

class HexBedConfiguration : public Configuration {
  public:
    void load();
    void save();
    void apply();

  protected:
    bool loadBool(const string& key, bool def);
    long loadInt(const string& key, long def);
    float loadFloat(const string& key, double def);
    string loadString(const string& key, const string& def);

    void saveBool(const string& key, bool value);
    void saveInt(const string& key, long value);
    void saveFloat(const string& key, double value);
    void saveString(const string& key, const string& value);

  private:
    std::unordered_map<string, bool> loadedBool_;
    std::unordered_map<string, long> loadedInt_;
    std::unordered_map<string, double> loadedFloat_;
    std::unordered_map<string, string> loadedString_;
    std::basic_ostream<strchar>* outStream_;
};

extern HexBedConfiguration currentConfig;
inline const ConfigurationValues& config() { return currentConfig.values(); }
inline ConfigurationValues& configValues() { return currentConfig.values(); }

};  // namespace hexbed

#endif /* HEXBED_APP_CONFIG_HH */
