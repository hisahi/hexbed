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
// plugins/inspector/uint32.hh -- header for builtin uint32 inspector

#ifndef HEXBED_PLUGINS_INSPECTOR_UINT32_HH
#define HEXBED_PLUGINS_INSPECTOR_UINT32_HH

#include "plugins/inspector.hh"

namespace hexbed {

namespace plugins {

class InspectorPluginUInt32 : public LocalizableDataInspectorPlugin {
  public:
    InspectorPluginUInt32(pluginid id);
    bool convertFromBytes(std::size_t outstr_n, char* outstr,
                          const_bytespan data,
                          const DataInspectorSettings& settings);
    bool convertToBytes(std::size_t& outdata_n, byte* outdata,
                        const char* instr,
                        const DataInspectorSettings& settings);
};

};  // namespace plugins

};  // namespace hexbed

#endif /* HEXBED_PLUGINS_INSPECTOR_UINT32_HH */
