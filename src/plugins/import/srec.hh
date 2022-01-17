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
// plugins/import/srec.hh -- header for builtin Motorola S-record importer

#ifndef HEXBED_PLUGINS_IMPORT_SREC_HH
#define HEXBED_PLUGINS_IMPORT_SREC_HH

#include "plugins/import.hh"

namespace hexbed {

namespace plugins {

struct ImportPluginMotorolaSRECSettings {
    bool checksums{true};
};

class ImportPluginMotorolaSREC : public LocalizableImportPlugin {
  public:
    ImportPluginMotorolaSREC(pluginid id);
    wxString getFileFilter() const;
    bool configureImport(wxWindow* parent,
                         const std::filesystem::path& filename);
    void doImport(HexBedTask& task, const std::filesystem::path& filename,
                  std::function<void(bufsize, const_bytespan)> output);

  private:
    ImportPluginMotorolaSRECSettings settings_;
};

};  // namespace plugins

};  // namespace hexbed

#endif /* HEXBED_PLUGINS_IMPORT_SREC_HH */
