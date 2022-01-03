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
// ui/logger.hh -- header for the HexBed logger impl

#ifndef HEXBED_UI_LOGGER_HH
#define HEXBED_UI_LOGGER_HH

#include "common/logger.hh"

namespace hexbed {

class StdLogHandler : public LogHandler {
  public:
    inline StdLogHandler(LogLevel minimumLevel) : minimumLevel_(minimumLevel) {}
    void handle(LogLevel level, const std::tm& tm, const char* file,
                size_t line, const std::string& msg) override;
    ~StdLogHandler() {}

  private:
    LogLevel minimumLevel_;
};

};  // namespace hexbed

#endif /* HEXBED_UI_LOGGER_HH */
