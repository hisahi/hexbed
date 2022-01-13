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
// ui/plugins/inspector/float64.cc -- impl for builtin double inspector

#include "ui/plugins/inspector/float64.hh"

#include <wx/string.h>
#include <wx/translation.h>

#include <cfloat>
#include <cstdio>

#include "common/floatconv.hh"
#include "common/intconv.hh"
#include "common/logger.hh"

namespace hexbed {

namespace plugins {

#define STRINGIFY_(x) #x
#define STRINGIFY(x) STRINGIFY_(x)

InspectorPluginFloat64::InspectorPluginFloat64(pluginid id)
    : LocalizableDataInspectorPlugin(
          id, TAG("double (IEEE 754 binary64 double-precision floating-point)"),
          sizeof(double), 64) {
    static_assert(sizeof(double) == 8);
    static_assert(std::numeric_limits<double>::is_iec559);
}

bool InspectorPluginFloat64::convertFromBytes(
    std::size_t outstr_n, char* outstr, const_bytespan data,
    const DataInspectorSettings& settings) {
    std::uint64_t result = uintFromBytes<std::uint64_t>(
        data.size(), data.data(), settings.littleEndian);
    double num = int64ToFloat64(result);
    std::snprintf(outstr, outstr_n, "%." STRINGIFY(DBL_DIG) "lg", num);
    return true;
}

bool InspectorPluginFloat64::convertToBytes(
    std::size_t& outdata_n, byte* outdata, const char* instr,
    const DataInspectorSettings& settings) {
    const char* endptr;
    double f = strtod(instr, const_cast<char**>(&endptr));
    if (instr == endptr) return false;
    std::uint64_t result = float64ToInt64(f);
    HEXBED_ASSERT(outdata_n >= 1);
    outdata_n = uintToBytes<std::uint64_t>(outdata_n, outdata, result,
                                           settings.littleEndian);
    return true;
}

};  // namespace plugins

};  // namespace hexbed
