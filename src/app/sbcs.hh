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
// app/sbcs.hh -- header for charset plugins

#ifndef HEXBED_APP_SBCS_HH
#define HEXBED_APP_SBCS_HH

#include <utility>

#include "app/plugin.hh"
#include "common/charconv.hh"

namespace hexbed {

namespace plugins {

struct CharsetPluginNamesIterator;

void loadCharsetPluginFrom(const std::filesystem::path& p);
std::size_t charsetPluginCount();
const std::pair<string, string>& charsetPluginByIndex(std::size_t);

};  // namespace plugins

SingleByteCharacterSet getSbcsByName(const string& name);

};  // namespace hexbed

#endif /* HEXBED_APP_SBCS_HH */
