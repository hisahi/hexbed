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
// common/logger.cc -- used to implement logging

#include "common/logger.hh"

namespace hexbed {

std::string exceptionAsString(const std::exception_ptr& eptr) {
    if (!eptr) return "<no error>";
    try {
        std::rethrow_exception(eptr);
    } catch (const std::exception& e) {
        return e.what();
    } catch (const std::string& e) {
        return e;
    } catch (const char* e) {
        return e;
    } catch (...) {
        return "<unknown error>";
    }
};

static std::tm* s_localtime(const std::time_t* time) {
#if _POSIX_C_SOURCE >= 1 || _XOPEN_SOURCE || _BSD_SOURCE || _SVID_SOURCE || \
    _POSIX_SOURCE
    static std::tm buf;
    return localtime_r(time, &buf);
#elif defined(_MSC_VER)
    static std::tm buf;
    size_t e = localtime_s(&buf, time);
    return e ? NULL : &buf;
#else
    return std::localtime(&time);
#endif
}

void Logger::log_(LogLevel level, const char* file, size_t line,
                  const std::string& s) {
    std::time_t t_now = std::time(nullptr);
    std::tm tm_now = *s_localtime(&t_now);
    for (auto& handler : handlers_)
        handler->handle(level, tm_now, file, line, s);
}

Logger logger;
bool logger_ok = false;

};  // namespace hexbed
