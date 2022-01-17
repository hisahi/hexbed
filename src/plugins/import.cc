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
// plugins/import.cc -- impl for the import plugin system

#include "plugins/import.hh"

#include <wx/intl.h>

#include <memory>
#include <vector>

#include "plugins/import/intelhex.hh"
#include "plugins/import/srec.hh"

namespace hexbed {

namespace plugins {

static std::vector<std::unique_ptr<ImportPlugin>> importPlugins;

void loadBuiltinImportPlugins() {
    importPlugins.push_back(
        std::make_unique<ImportPluginIntelHEX>(nextBuiltinPluginId()));
    importPlugins.push_back(
        std::make_unique<ImportPluginMotorolaSREC>(nextBuiltinPluginId()));
}

std::size_t importPluginCount() { return importPlugins.size(); }

ImportPlugin& importPluginByIndex(std::size_t i) {
    return *importPlugins.at(i);
}

wxString ImportPlugin::getFileFilter() const {
    return _("All files (*.*)") + "|*";
}

};  // namespace plugins

};  // namespace hexbed
