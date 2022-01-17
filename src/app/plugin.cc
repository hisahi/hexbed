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
// app/plugin.cc -- impl for the generic plugin system

#include "app/plugin.hh"

#include <filesystem>

#include "app/encoding.hh"
#include "common/ctype.hh"
#include "common/logger.hh"
#include "plugins/export.hh"
#include "plugins/import.hh"
#include "plugins/inspector.hh"

namespace hexbed {

namespace plugins {

std::filesystem::path executableDirectory;

static bool caseInsCheckChar(strchar a, strchar b) {
    return c_toupper(a) == c_toupper(b);
}

static bool caseInsCheck(const strchar* f, const strchar* s) {
    while (*f && *s)
        if (!caseInsCheckChar(*f++, *s++)) return false;
    return !*f && !*s;
}

bool checkCaseInsensitiveFileExtension(const std::filesystem::path& fn,
                                       const string& suffix) {
    auto ext = fn.extension();
    return caseInsCheck(ext.c_str(), suffix.c_str());
}

static void loadExternalPluginFilesFrom(
    const std::filesystem::path& p, const string& fileExt,
    std::function<void(const std::filesystem::path&)> load) {
    try {
        for (const auto& dir_entry : std::filesystem::directory_iterator{p}) {
            const std::filesystem::path& p = dir_entry.path();
            if (checkCaseInsensitiveFileExtension(p, fileExt)) load(p);
        }
    } catch (const std::filesystem::filesystem_error& e) {
        LOG_DEBUG("could not load plugin files from %" FMT_STR ": %s",
                  p.native(), currentExceptionAsString());
    }
}

static void loadExternalPluginsFrom(const std::filesystem::path& p) {
    LOG_DEBUG("loading plugins from <%" FMT_STR ">", p.native());
    loadExternalPluginFilesFrom(p / STRING("charsets"), STRING(".set"),
                                loadCharsetPluginFrom);
}

void loadExternalPlugins() {
    loadExternalPluginsFrom(executableDirectory / STRING("plugins"));
}

void loadBuiltinPlugins() {
    loadBuiltinDataInspectorPlugins();
    loadBuiltinImportPlugins();
    loadBuiltinExportPlugins();
}

pluginid nextPluginId_ = 0;
pluginid firstPluginIdCustom_ = 0;
pluginid nextPluginIdCustom_ = 0;

pluginid nextBuiltinPluginId() { return nextPluginId_++; }

void resetExternalPluginIds() { nextPluginIdCustom_ = firstPluginIdCustom_; }

pluginid nextExternalPluginId() {
    if (firstPluginIdCustom_ < nextPluginId_)
        firstPluginIdCustom_ = nextPluginId_;
    if (nextPluginIdCustom_ < firstPluginIdCustom_)
        nextPluginIdCustom_ = firstPluginIdCustom_;
    return nextPluginIdCustom_++;
}

};  // namespace plugins

};  // namespace hexbed
