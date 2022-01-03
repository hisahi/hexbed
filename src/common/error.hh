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
// common/error.hh -- header for error handling

#ifndef HEXBED_COMMON_ERROR_HH
#define HEXBED_COMMON_ERROR_HH

#include <cerrno>
#include <cstring>
#include <stdexcept>

namespace hexbed {

class system_io_error : public std::runtime_error {
    using std::runtime_error::runtime_error;
};

inline system_io_error errno_to_exception(int e = errno) {
    return system_io_error(e ? std::strerror(e) : "unknown error");
}

};  // namespace hexbed

#endif /* HEXBED_COMMON_FORMAT_HH */
