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
// common/format.hh -- header for string formatting

#ifndef HEXBED_COMMON_FORMAT_HH
#define HEXBED_COMMON_FORMAT_HH

#include <cstdio>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>

#include "common/types.hh"

namespace hexbed {

template <typename T>
struct c_ify_return {
    using type = T;
};
template <>
struct c_ify_return<std::string> {
    using type = const char*;
};

template <typename T>
typename c_ify_return<T>::type c_ify(const T& x) {
    static_assert(std::is_fundamental_v<T> || std::is_pointer_v<T>);
    return x;
}

template <>
inline typename c_ify_return<std::string>::type c_ify<std::string>(
    const std::string& s) {
    return s.c_str();
}

template <>
inline typename c_ify_return<std::wstring>::type c_ify<std::wstring>(
    const std::wstring& s) {
    return s.c_str();
}

template <typename... Ts>
std::string stringFormat(const std::string& fmt, const Ts&... args) {
    int n = std::snprintf(nullptr, 0, fmt.c_str(), c_ify<Ts>(args)...);
    if (n < 0) throw std::runtime_error("failed to format");
    std::string s(n, ' ');
    std::snprintf(s.data(), n + 1, fmt.c_str(), c_ify<Ts>(args)...);
    return s;
}

template <typename... Ts>
string xstringFormat(const string& fmt, const Ts&... args) {
    if constexpr (std::is_same_v<string, std::wstring>) {
        int n = std::swprintf(nullptr, 0, fmt.c_str(), c_ify<Ts>(args)...);
        if (n < 0) throw std::runtime_error("failed to format");
        string s(n, ' ');
        std::swprintf(s.data(), n + 1, fmt.c_str(), c_ify<Ts>(args)...);
        return s;
    } else {
        int n = std::snprintf(nullptr, 0, fmt.c_str(), c_ify<Ts>(args)...);
        if (n < 0) throw std::runtime_error("failed to format");
        string s(n, ' ');
        std::snprintf(s.data(), n + 1, fmt.c_str(), c_ify<Ts>(args)...);
        return s;
    }
}

extern const char* const HEX_UPPERCASE;
extern const char* const HEX_LOWERCASE;

};  // namespace hexbed

#endif /* HEXBED_COMMON_FORMAT_HH */
