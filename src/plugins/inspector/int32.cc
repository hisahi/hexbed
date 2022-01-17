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
// plugins/inspector/int32.cc -- impl for builtin int32 inspector

#include "plugins/inspector/int32.hh"

#include <wx/string.h>
#include <wx/translation.h>

#include <cstdint>

#include "common/intconv.hh"
#include "common/limits.hh"
#include "common/logger.hh"

namespace hexbed {

namespace plugins {

InspectorPluginInt32::InspectorPluginInt32(pluginid id)
    : LocalizableDataInspectorPlugin(id, TAG("int32 (signed 32-bit integer)"),
                                     sizeof(std::int32_t),
                                     2 + maxDecimalDigits<std::int32_t>()) {}

bool InspectorPluginInt32::convertFromBytes(
    std::size_t outstr_n, char* outstr, const_bytespan data,
    const DataInspectorSettings& settings) {
    std::int32_t result = intFromBytes<std::int32_t>(data.size(), data.data(),
                                                     settings.littleEndian);
    intToString<std::int32_t>(outstr_n, outstr, result);
    return true;
}

bool InspectorPluginInt32::convertToBytes(
    std::size_t& outdata_n, byte* outdata, const char* instr,
    const DataInspectorSettings& settings) {
    std::int32_t result;
    HEXBED_ASSERT(outdata_n >= 1);
    if (intFromString<std::int32_t>(result, instr)) {
        outdata_n = intToBytes<std::int32_t>(outdata_n, outdata, result,
                                             settings.littleEndian);
        return true;
    }
    return false;
}

};  // namespace plugins

};  // namespace hexbed
