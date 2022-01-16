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
    bool littleEndian;

    DataInspectorSettings();
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
    inline bool isReadOnly() const noexcept { return readOnly_; }
    inline bool isLocalizable() const noexcept { return localizable_; }
    virtual bool convertFromBytes(std::size_t outstr_n, strchar* outstr,
                                  const_bytespan data,
                                  const DataInspectorSettings& settings) = 0;
    virtual bool convertToBytes(std::size_t& outdata_n, byte* outdata,
                                const strchar* instr,
                                const DataInspectorSettings& settings) = 0;
    inline bool convertToBytes(std::size_t& outdata_n, byte* outdata,
                               const string& instr,
                               const DataInspectorSettings& settings) {
        return convertToBytes(outdata_n, outdata, instr.c_str(), settings);
    }

    virtual inline ~DataInspectorPlugin() {}

  protected:
    inline DataInspectorPlugin(pluginid id, const wxString& title,
                               std::size_t maxDataLen, std::size_t maxStrLen)
        : DataInspectorPlugin(id, title, false, false, maxDataLen, maxStrLen) {}

    inline DataInspectorPlugin(pluginid id, const wxString& title,
                               bool readOnly, bool localizable,
                               std::size_t maxDataLen, std::size_t maxStrLen)
        : Plugin(id),
          title_(title),
          readOnly_(readOnly),
          localizable_(localizable),
          maxDataLen_(maxDataLen),
          maxStrLen_(maxStrLen) {}

  private:
    wxString title_;
    bool readOnly_;
    bool localizable_;
    std::size_t maxDataLen_;
    std::size_t maxStrLen_;
};

class LocalizableDataInspectorPlugin : public DataInspectorPlugin {
  protected:
    inline LocalizableDataInspectorPlugin(pluginid id, const wxString& title,
                                          std::size_t maxDataLen,
                                          std::size_t maxStrLen)
        : DataInspectorPlugin(id, title, false, true, maxDataLen, maxStrLen) {}
};

class ReadOnlyDataInspectorPlugin : public DataInspectorPlugin {
  public:
    inline bool convertToBytes(std::size_t&, byte*, const char*,
                               const DataInspectorSettings&) {
        return false;
    }

  protected:
    inline ReadOnlyDataInspectorPlugin(pluginid id, const wxString& title,
                                       std::size_t maxDataLen,
                                       std::size_t maxStrLen)
        : DataInspectorPlugin(id, title, true, false, maxDataLen, maxStrLen) {}
};

class ReadOnlyLocalizableDataInspectorPlugin : public DataInspectorPlugin {
  public:
    inline bool convertToBytes(std::size_t&, byte*, const char*,
                               const DataInspectorSettings&) {
        return false;
    }

  protected:
    inline ReadOnlyLocalizableDataInspectorPlugin(pluginid id,
                                                  const wxString& title,
                                                  std::size_t maxDataLen,
                                                  std::size_t maxStrLen)
        : DataInspectorPlugin(id, title, true, true, maxDataLen, maxStrLen) {}
};

void loadBuiltinDataInspectorPlugins();
std::size_t dataInspectorPluginCount();
DataInspectorPlugin& dataInspectorPluginByIndex(std::size_t);

};  // namespace plugins

};  // namespace hexbed

#endif /* HEXBED_UI_PLUGINS_INSPECTOR_HH */
