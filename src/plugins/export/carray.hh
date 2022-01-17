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
// plugins/export/carray.hh -- header for builtin C source code exporter

#ifndef HEXBED_PLUGINS_EXPORT_CARRAY_HH
#define HEXBED_PLUGINS_EXPORT_CARRAY_HH

#include "plugins/export.hh"

namespace hexbed {

namespace plugins {

struct ExportPluginCSettings {
    std::string variableName{"data"};
    bool signedChar{false};
    bool stdint{false};
    unsigned columns{80};
    unsigned bytesPerLine{16};
    bool useBytesPerLine{false};
    unsigned indentSpace{4};
    bool indentTabs{false};
    bool useHex{true};
    bool isStatic{false};
};

class ExportPluginC : public LocalizableExportPlugin {
  public:
    ExportPluginC(pluginid id);
    wxString getFileFilter() const;
    bool configureExport(wxWindow* parent,
                         const std::filesystem::path& filename,
                         bufsize actualOffset, bufsize size,
                         const ExportDetails& details);
    void doExport(HexBedTask& task, const std::filesystem::path& filename,
                  std::function<bufsize(bufsize, bytespan)> read,
                  bufsize actualOffset, bufsize size,
                  const ExportDetails& details);

  private:
    ExportPluginCSettings settings_;
};

};  // namespace plugins

};  // namespace hexbed

#endif /* HEXBED_PLUGINS_EXPORT_CARRAY_HH */
