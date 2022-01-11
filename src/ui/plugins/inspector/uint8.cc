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
// ui/plugins/inspector/uint8.cc -- impl for builtin uint8 inspector

#include "ui/plugins/inspector/uint8.hh"

#include <wx/string.h>
#include <wx/translation.h>

#include <cstdint>

#include "common/intconv.hh"
#include "common/limits.hh"
#include "common/logger.hh"

namespace hexbed {

namespace plugins {

InspectorPluginUInt8::InspectorPluginUInt8(pluginid id)
    : DataInspectorPlugin(id, _("uint8 (unsigned 8-bit integer)"),
                          sizeof(std::uint8_t),
                          1 + maxDecimalDigits<std::uint8_t>()) {}

bool InspectorPluginUInt8::convertFromBytes(
    std::size_t outstr_n, char* outstr, const_bytespan data,
    const DataInspectorSettings& settings) {
    std::uint8_t result = uintFromBytes<std::uint8_t>(data.size(), data.data(),
                                                      settings.littleEndian);
    uintToString<std::uint8_t>(outstr_n, outstr, result);
    return true;
}

bool InspectorPluginUInt8::convertToBytes(
    std::size_t& outdata_n, byte* outdata, const char* instr,
    const DataInspectorSettings& settings) {
    std::uint8_t result;
    HEXBED_ASSERT(outdata_n >= 1);
    if (uintFromString<std::uint8_t>(result, instr)) {
        outdata_n = uintToBytes<std::uint8_t>(outdata_n, outdata, result,
                                              settings.littleEndian);
        return true;
    }
    return false;
}

};  // namespace plugins

};  // namespace hexbed
