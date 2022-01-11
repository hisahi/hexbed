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
// ui/plugins/plugin.cc -- impl for the generic plugin system

#include "ui/plugins/plugin.hh"

#include "ui/plugins/inspector.hh"

namespace hexbed {

namespace plugins {

void loadBuiltinPlugins() {
    loadBuiltinDataInspectorPlugins();
    // ...
}

pluginid nextPluginId_ = 0;
pluginid firstPluginIdCustom_ = 0;
pluginid nextPluginIdCustom_ = 0;

pluginid nextBuiltinPluginId() { return nextPluginId_++; }

void resetExternalPluginIds() { nextPluginIdCustom_ = firstPluginIdCustom_; }

pluginid nextExternalPluginId() {
    if (firstPluginIdCustom_ < nextPluginId_)
        firstPluginIdCustom_ = nextPluginId_;
    if (nextPluginIdCustom_ < firstPluginIdCustom_)
        nextPluginIdCustom_ = firstPluginIdCustom_;
    return nextPluginIdCustom_++;
}

};  // namespace plugins

};  // namespace hexbed
