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
// ui/plugins/inspector.hh -- header for the data inspector plugin system

#ifndef HEXBED_UI_PLUGINS_INSPECTOR_HH
#define HEXBED_UI_PLUGINS_INSPECTOR_HH

#include <wx/string.h>

#include "common/types.hh"
#include "ui/plugins/plugin.hh"

namespace hexbed {

namespace plugins {

struct DataInspectorSettings {
    bool littleEndian{false};
};

class DataInspectorPlugin : public Plugin {
  public:
    inline const wxString& getTitle() const noexcept { return title_; }
    inline std::size_t getRequestedDataBufferSize() const noexcept {
        return maxDataLen_;
    }
    inline std::size_t getRequestedStringBufferSize() const noexcept {
        return maxStrLen_;
    }
    virtual bool convertFromBytes(std::size_t outstr_n, char* outstr,
                                  const_bytespan data,
                                  const DataInspectorSettings& settings) = 0;
    virtual bool convertToBytes(std::size_t& outdata_n, byte* outdata,
                                const char* instr,
                                const DataInspectorSettings& settings) = 0;

  protected:
    inline DataInspectorPlugin(pluginid id, const wxString& title,
                               std::size_t maxDataLen, std::size_t maxStrLen)
        : Plugin(id),
          title_(title),
          maxDataLen_(maxDataLen),
          maxStrLen_(maxStrLen) {}

  private:
    wxString title_;
    std::size_t maxDataLen_;
    std::size_t maxStrLen_;
};

void loadBuiltinDataInspectorPlugins();
std::size_t dataInspectorPluginCount();
DataInspectorPlugin& dataInspectorPluginByIndex(std::size_t);

};  // namespace plugins

};  // namespace hexbed

#endif /* HEXBED_UI_PLUGINS_INSPECTOR_HH */