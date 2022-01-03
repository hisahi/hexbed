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
// common/charconv.hh -- header for character conversions

#ifndef HEXBED_COMMON_CHARCONV_HH
#define HEXBED_COMMON_CHARCONV_HH

#include <array>
#include <bitset>
#include <string>

#include "common/types.hh"

namespace hexbed {

class SingleByteCharacterSet {
  public:
    SingleByteCharacterSet();
    SingleByteCharacterSet(std::array<char32_t, 256> map);
    SingleByteCharacterSet(std::u32string str);

    // 0-255 or -1 for not allowed
    int fromChar(char32_t u);
    char32_t toChar(byte b);
    // only give printable characters, or 0 if not printable
    char32_t toPrintableChar(byte b);

  private:
    void initPrint();
    std::array<char32_t, 256> map_;
    std::bitset<256> print_;
};

extern SingleByteCharacterSet sbcs;

SingleByteCharacterSet getSbcsByName(const std::string& name);

std::wstring sbcsFromBytes(bufsize len, const byte* data);
bool sbcsToBytes(bufsize& len, byte* data, const std::wstring& text);

};  // namespace hexbed

#endif /* HEXBED_COMMON_CHARCONV_HH */
