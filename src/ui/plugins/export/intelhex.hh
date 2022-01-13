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
// ui/plugins/export/intelhex.hh -- header for builtin Intel HEX exporter

#ifndef HEXBED_UI_PLUGINS_EXPORT_INTELHEX_HH
#define HEXBED_UI_PLUGINS_EXPORT_INTELHEX_HH

#include "ui/plugins/export.hh"

namespace hexbed {

namespace plugins {

struct ExportPluginIntelHEXSettings {
    unsigned columns{16};
    bool segmentMode{false};
};

class ExportPluginIntelHEX : public LocalizableExportPlugin {
  public:
    ExportPluginIntelHEX(pluginid id);
    wxString getFileFilter() const;
    bool configureExport(wxWindow* parent, const std::string& filename,
                         bufsize size);
    void doExport(HexBedTask& task, const std::string& filename,
                  std::function<bufsize(bufsize, bytespan)> read,
                  bufsize actualOffset, bufsize size);

  private:
    ExportPluginIntelHEXSettings settings_;
};

};  // namespace plugins

};  // namespace hexbed

#endif /* HEXBED_UI_PLUGINS_EXPORT_INTELHEX_HH */
