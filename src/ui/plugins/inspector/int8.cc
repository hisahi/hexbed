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
// ui/plugins/inspector/int8.cc -- impl for builtin int8 inspector

#include "ui/plugins/inspector/int8.hh"

#include <wx/string.h>
#include <wx/translation.h>

#include <cstdint>

#include "common/intconv.hh"
#include "common/limits.hh"
#include "common/logger.hh"

namespace hexbed {

namespace plugins {

InspectorPluginInt8::InspectorPluginInt8(pluginid id)
    : DataInspectorPlugin(id, _("int8 (signed 8-bit integer)"),
                          sizeof(std::int8_t),
                          2 + maxDecimalDigits<std::int8_t>()) {}

bool InspectorPluginInt8::convertFromBytes(
    std::size_t outstr_n, char* outstr, const_bytespan data,
    const DataInspectorSettings& settings) {
    std::int8_t result = intFromBytes<std::int8_t>(data.size(), data.data(),
                                                   settings.littleEndian);
    intToString<std::int8_t>(outstr_n, outstr, result);
    return true;
}

bool InspectorPluginInt8::convertToBytes(
    std::size_t& outdata_n, byte* outdata, const char* instr,
    const DataInspectorSettings& settings) {
    std::int8_t result;
    HEXBED_ASSERT(outdata_n >= 1);
    if (intFromString<std::int8_t>(result, instr)) {
        outdata_n = intToBytes<std::int8_t>(outdata_n, outdata, result,
                                            settings.littleEndian);
        return true;
    }
    return false;
}

};  // namespace plugins

};  // namespace hexbed
