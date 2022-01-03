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
// ui/logger.cc -- the HexBed logger impl

#include "ui/logger.hh"

#include <iomanip>
#include <iostream>

namespace hexbed {

static std::string levelName(LogLevel level) noexcept {
    switch (level) {
    case LogLevel::TRACE:
        return "trac";
    case LogLevel::DEBUG:
        return "dbg ";
    case LogLevel::INFO:
        return "info";
    case LogLevel::WARN:
        return "WARN";
    case LogLevel::ERROR:
        return "ERR ";
    case LogLevel::FAIL:
        return "!!! ";
    }
    return "??? ";
}

constexpr const char iso8601[] = "%FT%T%z";

void StdLogHandler::handle(LogLevel level, const std::tm& tm, const char* file,
                           size_t line, const std::string& msg) {
    if (level >= minimumLevel_)
        std::cerr << "[" << levelName(level) << " : " << file << ":" << line
                  << " @ " << std::put_time(&tm, iso8601) << "] " << msg
                  << std::endl;
}

};  // namespace hexbed
