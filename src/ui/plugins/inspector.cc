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
// ui/plugins/inspector.cc -- impl for the data inspector plugin system

#include "ui/plugins/inspector.hh"

#include <bit>
#include <memory>
#include <vector>

#include "ui/plugins/inspector/float32.hh"
#include "ui/plugins/inspector/float32x.hh"
#include "ui/plugins/inspector/float64.hh"
#include "ui/plugins/inspector/float64x.hh"
#include "ui/plugins/inspector/int16.hh"
#include "ui/plugins/inspector/int32.hh"
#include "ui/plugins/inspector/int64.hh"
#include "ui/plugins/inspector/int8.hh"
#include "ui/plugins/inspector/uint16.hh"
#include "ui/plugins/inspector/uint32.hh"
#include "ui/plugins/inspector/uint64.hh"
#include "ui/plugins/inspector/uint8.hh"

namespace hexbed {

namespace plugins {

static std::vector<std::unique_ptr<DataInspectorPlugin>> dataInspectors;

DataInspectorSettings::DataInspectorSettings()
    : littleEndian(std::endian::native == std::endian::little) {}

void loadBuiltinDataInspectorPlugins() {
    dataInspectors.push_back(
        std::make_unique<InspectorPluginInt8>(nextBuiltinPluginId()));
    dataInspectors.push_back(
        std::make_unique<InspectorPluginInt16>(nextBuiltinPluginId()));
    dataInspectors.push_back(
        std::make_unique<InspectorPluginInt32>(nextBuiltinPluginId()));
    dataInspectors.push_back(
        std::make_unique<InspectorPluginInt64>(nextBuiltinPluginId()));
    dataInspectors.push_back(
        std::make_unique<InspectorPluginUInt8>(nextBuiltinPluginId()));
    dataInspectors.push_back(
        std::make_unique<InspectorPluginUInt16>(nextBuiltinPluginId()));
    dataInspectors.push_back(
        std::make_unique<InspectorPluginUInt32>(nextBuiltinPluginId()));
    dataInspectors.push_back(
        std::make_unique<InspectorPluginUInt64>(nextBuiltinPluginId()));
    dataInspectors.push_back(
        std::make_unique<InspectorPluginFloat32>(nextBuiltinPluginId()));
    dataInspectors.push_back(
        std::make_unique<InspectorPluginFloat64>(nextBuiltinPluginId()));
    dataInspectors.push_back(
        std::make_unique<InspectorPluginFloat32Hex>(nextBuiltinPluginId()));
    dataInspectors.push_back(
        std::make_unique<InspectorPluginFloat64Hex>(nextBuiltinPluginId()));
}

std::size_t dataInspectorPluginCount() { return dataInspectors.size(); }

DataInspectorPlugin& dataInspectorPluginByIndex(std::size_t i) {
    return *dataInspectors.at(i);
}

};  // namespace plugins

};  // namespace hexbed
