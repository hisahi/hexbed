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
// ui/plugins/export.hh -- header for the export plugin system

#ifndef HEXBED_UI_PLUGINS_EXPORT_HH
#define HEXBED_UI_PLUGINS_EXPORT_HH

#include <wx/string.h>
#include <wx/window.h>

#include <filesystem>
#include <functional>

#include "common/types.hh"
#include "file/task.hh"
#include "ui/plugins/plugin.hh"

namespace hexbed {

namespace plugins {

class ExportPlugin : public Plugin {
  public:
    inline const wxString& getTitle() const noexcept { return title_; }
    inline bool isLocalizable() const noexcept { return localizable_; }
    virtual wxString getFileFilter() const;

    virtual bool configureExport(wxWindow* parent,
                                 const std::filesystem::path& filename,
                                 bufsize actualOffset, bufsize size) = 0;
    virtual void doExport(HexBedTask& task,
                          const std::filesystem::path& filename,
                          std::function<bufsize(bufsize, bytespan)> read,
                          bufsize actualOffset, bufsize size) = 0;

  protected:
    inline ExportPlugin(pluginid id, const wxString& title,
                        std::size_t maxDataLen, std::size_t maxStrLen)
        : ExportPlugin(id, title, false) {}

    inline ExportPlugin(pluginid id, const wxString& title, bool localizable)
        : Plugin(id), title_(title), localizable_(localizable) {}

  private:
    wxString title_;
    bool readOnly_;
    bool localizable_;
};

class LocalizableExportPlugin : public ExportPlugin {
  protected:
    inline LocalizableExportPlugin(pluginid id, const wxString& title)
        : ExportPlugin(id, title, true) {}
};

void loadBuiltinExportPlugins();
std::size_t exportPluginCount();
ExportPlugin& exportPluginByIndex(std::size_t);

};  // namespace plugins

};  // namespace hexbed

#endif /* HEXBED_UI_PLUGINS_EXPORT_HH */
