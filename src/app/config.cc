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
// app/config.cc -- implements main configuration file

#include "app/config.hh"

#include <cfloat>
#include <charconv>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <stdexcept>

#include "app/encoding.hh"
#include "common/charconv.hh"
#include "common/logger.hh"
#include "common/types.hh"

namespace hexbed {

#if (HAVE_UNISTD_H || defined(__unix__) || defined(__unix) || \
     defined(__QNX__) || (defined(__APPLE__) && defined(__MACH__)))
#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200809L
#endif
#include <unistd.h>
#endif

#ifdef __ANDROID__
#define OS_ANDROID 1
#endif

#if defined(__unix__) || (defined(__APPLE__) && defined(__MACH__))
#if defined(_POSIX_VERSION)
#define OS_POSIX 1
#endif
#if defined(__unix__)
#define OS_UNIX 1
#include <sys/param.h>
#if defined(__linux__)
#define OS_LINUX 1
#elif defined(BSD)
#define OS_BSD 1
#endif

#elif defined(__APPLE__)
#define OS_APPLE 1
#if defined(_POSIX_VERSION)
#define OS_POSIX 1
#endif
#include <TargetConditionals.h>
#include <sys/syslimits.h>
#if TARGET_OS_IPHONE
#define OS_IOS 1
#elif TARGET_OS_MAC
#define OS_MACOS 1
#endif
#endif

#elif defined(_WIN32) || defined(__CYGWIN__)
#define OS_WINDOWS 1
#if defined(__CYGWIN__)
#define OS_CYGWIN 1
#endif

#elif defined(_POSIX_VERSION)
#define OS_POSIX 1
#endif

#if OS_POSIX
#include <sys/stat.h>
#if OS_LINUX
#include <pwd.h>
#include <sys/types.h>
#endif
#elif OS_MACOS
#include <CoreServices/CoreServices.h>
#elif OS_WINDOWS
#include <shlobj.h>
#include <versionhelpers.h>
#error not implemented on your platform
#endif

static std::filesystem::path getConfigPath() {
    std::filesystem::path cfgp;
#if OS_POSIX
    const char* home = std::getenv("XDG_CONFIG_HOME");
    if (!home) {
        home = std::getenv("HOME");
#if OS_LINUX
        if (!home) {
            struct passwd* pw = getpwuid(getuid());
            if (pw) home = pw->pw_dir;
        }
#endif
        if (!home) home = ".";
        cfgp = std::filesystem::path(home);
        cfgp /= STRING(".config");
    } else
        cfgp = std::filesystem::path(home);
    cfgp /= STRING("hexbed");
    cfgp /= STRING("hexbed.conf");
#elif OS_MACOS
    char home[PATH_MAX];
    FSRef ref;
    FSFindFolder(kUserDomain, kApplicationSupportFolderType, kCreateFolder,
                 &ref);
    FSRefMakePath(&ref, (UInt8*)&home, PATH_MAX);
    cfgp = std::filesystem::path(home) / STRING("hexbed.conf");
#elif OS_WINDOWS
    char adata[MAX_PATH];
    std::filesystem::path adatas;
    if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, 0, adata)))
        adatas = std::filesystem::path(adata);
    else
        adatas = ".";
    cfgp = std::filesystem::path(adatas) / STRING("hexbed");
    cfgp /= STRING("hexbed.conf");
#else
#error config path not implemented on your platform
#endif
    return cfgp.string();
}

class convert_error : public std::runtime_error {
    using std::runtime_error::runtime_error;
};

static bool stringToBool(const string& s) {
    int res;
    auto fc = std::from_chars(s.data(), s.data() + s.size(), res);
    if (fc.ec != std::errc()) throw convert_error("failed to convert to bool");
    return res != 0;
}

static long stringToInt(const string& s) {
    long res;
    auto fc = std::from_chars(s.data(), s.data() + s.size(), res);
    if (fc.ec != std::errc()) throw convert_error("failed to convert to int");
    return res;
}

static double stringToFloat(const string& s) {
    double res;
    auto fc = std::from_chars(s.data(), s.data() + s.size(), res);
    if (fc.ec != std::errc()) throw convert_error("failed to convert to float");
    return res;
}

static string stringFromBool(bool value) {
    return xstringFormat("%d", value ? 1 : 0);
}

static string stringFromInt(long value) { return xstringFormat("%ld", value); }

static string stringFromFloat(double value) {
    return xstringFormat("%.*lf", DECIMAL_DIG, value);
}

void HexBedConfiguration::load() {
    try {
        string cpath = getConfigPath();
        std::basic_ifstream<strchar> cfg(cpath, std::ios::in);
        cfg.exceptions(std::ios::badbit);
        string line;
        while (std::getline(cfg, line)) {
            if (line.empty()) continue;
            auto eq = line.find('=');
            if (eq == string::npos) continue;
            string key = line.substr(1, eq - 1);
            string value = line.substr(eq + 1);
            try {
                switch (line[0]) {
                case '?':
                    loadedBool_.insert_or_assign(key, stringToBool(value));
                    break;
                case '&':
                    loadedInt_.insert_or_assign(key, stringToInt(value));
                    break;
                case '#':
                    loadedFloat_.insert_or_assign(key, stringToFloat(value));
                    break;
                case '$':
                    loadedString_.insert_or_assign(key, value);
                    break;
                }
            } catch (const convert_error& exc) {
                LOG_WARN("invalid config value: %" FMT_STR " (%s)", key,
                         currentExceptionAsString());
            }
        }
    } catch (...) {
        LOG_WARN("could not (fully) load config: %s",
                 currentExceptionAsString());
    }
    loadValues();

    loadedBool_.clear();
    loadedInt_.clear();
    loadedFloat_.clear();
    loadedString_.clear();
    apply();
}

void HexBedConfiguration::save() {
    try {
        std::filesystem::path cpath = getConfigPath();
        std::filesystem::create_directories(cpath.parent_path());
        outStream_ = new std::basic_ofstream<strchar>(cpath, std::ios::out);
        outStream_->exceptions(std::ios::failbit | std::ios::badbit);
        saveValues();
        delete std::exchange(outStream_, nullptr);
    } catch (...) {
        LOG_ERROR("could not (fully) save config: %s",
                  currentExceptionAsString());
    }
}

void HexBedConfiguration::apply() { sbcs = getSbcsByName(values_.charset); }

bool HexBedConfiguration::loadBool(const string& key, bool def) {
    auto s = loadedBool_.find(key);
    if (s == loadedBool_.end()) return def;
    return s->second;
}

long HexBedConfiguration::loadInt(const string& key, long def) {
    auto s = loadedInt_.find(key);
    if (s == loadedInt_.end()) return def;
    return s->second;
}

float HexBedConfiguration::loadFloat(const string& key, double def) {
    auto s = loadedFloat_.find(key);
    if (s == loadedFloat_.end()) return def;
    return s->second;
}

string HexBedConfiguration::loadString(const string& key, const string& def) {
    auto s = loadedString_.find(key);
    if (s == loadedString_.end()) return def;
    return s->second;
}

void HexBedConfiguration::saveBool(const string& key, bool value) {
    *outStream_ << "?" << key << "=" << stringFromBool(value) << '\n';
}

void HexBedConfiguration::saveInt(const string& key, long value) {
    *outStream_ << "&" << key << "=" << stringFromInt(value) << '\n';
}

void HexBedConfiguration::saveFloat(const string& key, double value) {
    *outStream_ << "#" << key << "=" << stringFromFloat(value) << '\n';
}

void HexBedConfiguration::saveString(const string& key, const string& value) {
    *outStream_ << "$" << key << "=" << value << '\n';
}

HexBedConfiguration currentConfig;

};  // namespace hexbed
