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
// ui/plugins/inspector/uint64.cc -- impl for builtin uint64 inspector

#include "ui/plugins/inspector/uint64.hh"

#include <wx/string.h>
#include <wx/translation.h>

#include <cstdint>

#include "common/intconv.hh"
#include "common/limits.hh"
#include "common/logger.hh"

namespace hexbed {

namespace plugins {

InspectorPluginUInt64::InspectorPluginUInt64(pluginid id)
    : LocalizableDataInspectorPlugin(
          id, TAG("uint64 (unsigned 64-bit integer)"), sizeof(std::uint64_t),
          1 + maxDecimalDigits<std::uint64_t>()) {}

bool InspectorPluginUInt64::convertFromBytes(
    std::size_t outstr_n, char* outstr, const_bytespan data,
    const DataInspectorSettings& settings) {
    std::uint64_t result = uintFromBytes<std::uint64_t>(
        data.size(), data.data(), settings.littleEndian);
    uintToString<std::uint64_t>(outstr_n, outstr, result);
    return true;
}

bool InspectorPluginUInt64::convertToBytes(
    std::size_t& outdata_n, byte* outdata, const char* instr,
    const DataInspectorSettings& settings) {
    std::uint64_t result;
    HEXBED_ASSERT(outdata_n >= 1);
    if (uintFromString<std::uint64_t>(result, instr)) {
        outdata_n = uintToBytes<std::uint64_t>(outdata_n, outdata, result,
                                               settings.littleEndian);
        return true;
    }
    return false;
}

};  // namespace plugins

};  // namespace hexbed
