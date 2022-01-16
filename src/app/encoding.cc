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
// app/encoding.cc -- impl for charset and char encoding plugins

#include "app/encoding.hh"

#include <array>
#include <fstream>
#include <string>
#include <unordered_map>
#include <vector>

#include "common/hexconv.hh"
#include "common/logger.hh"

namespace hexbed {

namespace plugins {

static std::vector<std::pair<string, string>> charsetPluginNames;
static std::unordered_map<string, SingleByteCharacterSet> charsetPlugins;

bool convertKeyFrom(const string& s, unsigned long& u) {
    long l;
    try {
        l = std::stol(s, nullptr, 0);
    } catch (const std::invalid_argument&) {
        return false;
    } catch (const std::out_of_range&) {
        return false;
    }
    if (l < 0) return false;
    u = static_cast<unsigned long>(l);
    return true;
}

bool convertValueFrom(const string& s, unsigned long& u) {
    return convertKeyFrom(s, u);
}

void loadCharsetPluginFrom(const std::filesystem::path& p) {
    string name;
    std::array<char32_t, 256> set = {0};
    try {
        std::basic_ifstream<strchar> cfg(p, std::ios::in);
        cfg.exceptions(std::ios::badbit);
        string line;
        while (std::getline(cfg, line)) {
            if (line.empty()) continue;
            auto eq = line.find_first_of(" \t");
            if (eq == string::npos) continue;
            string key = line.substr(0, eq);
            string value = line.substr(eq + 1);
            if (key.empty()) continue;
            if (key == STRING("name")) {
                name = value;
            } else if (decDigitToNum(key[0]) >= 0) {
                unsigned long nkey, nvalue;
                if (convertKeyFrom(key, nkey) &&
                    convertValueFrom(value, nvalue) && nkey < 256 &&
                    nvalue < UCHAR32_MAX) {
                    set[nkey] = static_cast<char32_t>(nvalue);
                }
            }
        }
    } catch (...) {
        LOG_WARN("could not load charset %" FMT_STR ": %s", p.native(),
                 currentExceptionAsString());
    }
    if (!name.empty()) {
        string sn = STRING("!");
        sn += p.stem();
        charsetPluginNames.emplace_back(sn, name);
        charsetPlugins.emplace(sn, SingleByteCharacterSet(set));
        LOG_DEBUG("loaded charset plugin '%" FMT_STR "' from <%" FMT_STR ">",
                  sn, p.native());
    } else {
        LOG_WARN("charset plugin in <%" FMT_STR "> has no name, skipping",
                 p.native());
    }
}

std::size_t charsetPluginCount() { return charsetPluginNames.size(); }

const std::pair<string, string>& charsetPluginByIndex(std::size_t i) {
    return charsetPluginNames[i];
}

};  // namespace plugins

SingleByteCharacterSet getSbcsByName(const string& name) {
    if (name.size() > 1 && name[0] == '!') {
        auto s = hexbed::plugins::charsetPlugins.find(name);
        if (s != hexbed::plugins::charsetPlugins.end()) return s->second;
    }
    return getBuiltinSbcsByName(name);
}

CharacterEncoding getCharacterEncodingByName(const string& name) {
    if (name.size() > 1 && name[0] == '!') {
        auto s = hexbed::plugins::charsetPlugins.find(name);
        if (s != hexbed::plugins::charsetPlugins.end())
            return CharacterEncoding(s->second);
    }
    return getBuiltinCharacterEncodingByName(name);
}

};  // namespace hexbed
