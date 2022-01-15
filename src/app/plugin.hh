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
// app/plugin.hh -- header for the generic plugin system

#ifndef HEXBED_APP_PLUGIN_HH
#define HEXBED_APP_PLUGIN_HH

#include <filesystem>

#include "common/types.hh"

namespace hexbed {

namespace plugins {

using pluginid = unsigned long;

class Plugin {
  public:
    inline pluginid id() const noexcept { return id_; }

  protected:
    inline Plugin(pluginid id) : id_(id) {}

  private:
    pluginid id_;
};

void loadBuiltinPlugins();
pluginid nextBuiltinPluginId();

void loadExternalPlugins();
void resetExternalPluginIds();
pluginid nextExternalPluginId();

extern std::filesystem::path executableDirectory;
bool checkCaseInsensitiveFileExtension(const std::filesystem::path& fn,
                                       const string& suffix);

};  // namespace plugins

};  // namespace hexbed

#endif /* HEXBED_APP_PLUGIN_HH */
