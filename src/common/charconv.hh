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

constexpr char32_t UCHAR32_MAX = 0x10FFFFUL;

class SingleByteCharacterSet {
  public:
    SingleByteCharacterSet();
    SingleByteCharacterSet(std::array<char32_t, 256> map);
    SingleByteCharacterSet(std::u32string str);

    // 0-255 or -1 for not allowed
    int fromChar(char32_t u) const;
    char32_t toChar(byte b) const;
    // only give printable characters, or 0 if not printable
    char32_t toPrintableChar(byte b) const;

  private:
    void initPrint();
    std::array<char32_t, 256> map_;
    std::bitset<256> print_;
};

extern SingleByteCharacterSet sbcs;

enum class TextEncoding { SBCS, UTF8, UTF16LE, UTF16BE, UTF32LE, UTF32BE };

SingleByteCharacterSet getBuiltinSbcsByName(const string& name);

std::u32string sbcsFromBytes(const SingleByteCharacterSet& sbcs, bufsize len,
                             const byte* data);
bool sbcsToBytes(const SingleByteCharacterSet& sbcs, bufsize& len, byte* data,
                 const std::u32string& text);

std::u32string sbcsFromBytes(bufsize len, const byte* data);
bool sbcsToBytes(bufsize& len, byte* data, const std::u32string& text);

struct DecodeStatus {
    bool ok{false};
    std::size_t charCount{0};
    std::size_t readCount{0};
};

using u32ostringstream = std::basic_ostringstream<char32_t>;

bool isPrintable(char32_t c);

std::size_t encodeCharMbcsOrSbcs(TextEncoding enc,
                                 const SingleByteCharacterSet& sbcs, char32_t c,
                                 std::size_t size, byte* out);
DecodeStatus decodeStringMbcsOrSbcs(TextEncoding enc,
                                    const SingleByteCharacterSet& sbcs,
                                    u32ostringstream& out, const_bytespan data);
std::u32string wstringToU32string(const std::wstring& w);
std::wstring u32stringToWstring(const std::u32string& w);

constexpr std::size_t MBCS_CHAR_MAX = 4;

char32_t convertCharFrom(unsigned utfMode, bufsize n, const byte* b,
                         bool printable = true);
std::u32string convertCharsFrom(unsigned utfMode, bufsize n, const byte* b,
                                bool printable = true);
bool convertCharsTo(unsigned utfMode, bufsize& len, byte* data,
                    const std::u32string& text);

};  // namespace hexbed

#endif /* HEXBED_COMMON_CHARCONV_HH */
