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
// ui/plugins/import.hh -- header for the import plugin system

#ifndef HEXBED_UI_PLUGINS_IMPORT_HH
#define HEXBED_UI_PLUGINS_IMPORT_HH

#include <wx/string.h>
#include <wx/window.h>

#include <functional>
#include <stdexcept>

#include "common/types.hh"
#include "file/task.hh"
#include "ui/plugins/plugin.hh"

namespace hexbed {

namespace plugins {

class ImportFileInvalidError : public std::runtime_error {
  public:
    ImportFileInvalidError(const char* c) : std::runtime_error(c) {}
    ImportFileInvalidError(const char* c, std::size_t l)
        : std::runtime_error(c), lineno_(l) {}

    bool lineKnown() const noexcept { return lineno_; }
    std::size_t lineNumber() const noexcept { return lineno_; }

  private:
    std::size_t lineno_{0};
};

class ImportPlugin : public Plugin {
  public:
    inline const wxString& getTitle() const noexcept { return title_; }
    inline bool isLocalizable() const noexcept { return localizable_; }
    virtual wxString getFileFilter() const;

    virtual bool configureImport(wxWindow* parent,
                                 const std::string& filename) = 0;
    virtual void doImport(
        HexBedTask& task, const std::string& filename,
        std::function<void(bufsize, const_bytespan)> output) = 0;

  protected:
    inline ImportPlugin(pluginid id, const wxString& title,
                        std::size_t maxDataLen, std::size_t maxStrLen)
        : ImportPlugin(id, title, false) {}

    inline ImportPlugin(pluginid id, const wxString& title, bool localizable)
        : Plugin(id), title_(title), localizable_(localizable) {}

  private:
    wxString title_;
    bool readOnly_;
    bool localizable_;
};

class LocalizableImportPlugin : public ImportPlugin {
  protected:
    inline LocalizableImportPlugin(pluginid id, const wxString& title)
        : ImportPlugin(id, title, true) {}
};

void loadBuiltinImportPlugins();
std::size_t importPluginCount();
ImportPlugin& importPluginByIndex(std::size_t);

};  // namespace plugins

};  // namespace hexbed

#endif /* HEXBED_UI_PLUGINS_IMPORT_HH */
