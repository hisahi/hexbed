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
// ui/plugins/plugin.hh -- header for the generic plugin system

#ifndef HEXBED_UI_PLUGINS_PLUGIN_HH
#define HEXBED_UI_PLUGINS_PLUGIN_HH

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
void resetExternalPluginIds();
pluginid nextExternalPluginId();

};  // namespace plugins

};  // namespace hexbed

#endif /* HEXBED_UI_PLUGINS_PLUGIN_HH */
