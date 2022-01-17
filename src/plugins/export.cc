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
// plugins/export.cc -- impl for the export plugin system

#include "plugins/export.hh"

#include <wx/intl.h>

#include <memory>
#include <vector>

#include "plugins/export/carray.hh"
#include "plugins/export/csharp.hh"
#include "plugins/export/html.hh"
#include "plugins/export/intelhex.hh"
#include "plugins/export/java.hh"
#include "plugins/export/srec.hh"
#include "plugins/export/text.hh"

namespace hexbed {

namespace plugins {

static std::vector<std::unique_ptr<ExportPlugin>> exportPlugins;

void loadBuiltinExportPlugins() {
    exportPlugins.push_back(
        std::make_unique<ExportPluginIntelHEX>(nextBuiltinPluginId()));
    exportPlugins.push_back(
        std::make_unique<ExportPluginMotorolaSREC>(nextBuiltinPluginId()));
    exportPlugins.push_back(
        std::make_unique<ExportPluginC>(nextBuiltinPluginId()));
    exportPlugins.push_back(
        std::make_unique<ExportPluginCSharp>(nextBuiltinPluginId()));
    exportPlugins.push_back(
        std::make_unique<ExportPluginJava>(nextBuiltinPluginId()));
    exportPlugins.push_back(
        std::make_unique<ExportPluginText>(nextBuiltinPluginId()));
    exportPlugins.push_back(
        std::make_unique<ExportPluginHTML>(nextBuiltinPluginId()));
}

std::size_t exportPluginCount() { return exportPlugins.size(); }

ExportPlugin& exportPluginByIndex(std::size_t i) {
    return *exportPlugins.at(i);
}

wxString ExportPlugin::getFileFilter() const {
    return _("All files (*.*)") + "|*";
}

};  // namespace plugins

};  // namespace hexbed
