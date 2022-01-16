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
#include <functional>
#include <string>
#include <variant>

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

SingleByteCharacterSet getBuiltinSbcsByName(const string& name);

std::u32string sbcsFromBytes(const SingleByteCharacterSet& sbcs, bufsize len,
                             const byte* data);
bool sbcsToBytes(const SingleByteCharacterSet& sbcs, bufsize& len, byte* data,
                 const std::u32string& text);

std::u32string sbcsFromBytes(bufsize len, const byte* data);
bool sbcsToBytes(bufsize& len, byte* data, const std::u32string& text);

using u32span = std::span<char32_t>;
using const_u32span = std::span<const char32_t>;

struct CharEncodeStatus {
    bool ok{false};
    bufsize readChars{0};
    bufsize wroteBytes{0};
};

struct CharDecodeStatus {
    bool ok{false};
    bufsize readBytes{0};
    bufsize wroteChars{0};
};

using CharEncodeInputFunction = std::function<bufsize(u32span)>;
using CharEncodeOutputFunction = std::function<void(const_bytespan)>;

using CharDecodeInputFunction = std::function<bufsize(bytespan)>;
using CharDecodeOutputFunction = std::function<void(const_u32span)>;

using CharEncodeFunction = std::function<CharEncodeStatus(
    CharEncodeInputFunction, CharEncodeOutputFunction)>;
using CharDecodeFunction = std::function<CharDecodeStatus(
    CharDecodeInputFunction, CharDecodeOutputFunction)>;

struct MultiByteCharacterSet {
    CharEncodeFunction encode;
    CharDecodeFunction decode;
};

class CharacterEncoding {
  public:
    CharacterEncoding(const SingleByteCharacterSet& sbcs);
    CharacterEncoding(const MultiByteCharacterSet& mbcs);
    CharEncodeStatus encode(CharEncodeInputFunction,
                            CharEncodeOutputFunction) const;
    CharDecodeStatus decode(CharDecodeInputFunction,
                            CharDecodeOutputFunction) const;

  private:
    std::variant<SingleByteCharacterSet, MultiByteCharacterSet> enc_;
};

MultiByteCharacterSet getBuiltinMbcsByName(const string& name);
CharacterEncoding getBuiltinCharacterEncodingByName(const string& name);

using u32ostringstream = std::basic_ostringstream<char32_t>;

CharEncodeInputFunction charEncodeFromArray(std::size_t n, const char32_t* arr);
CharEncodeInputFunction charEncodeFromString(std::u32string s);

CharEncodeOutputFunction charEncodeToNull();
CharEncodeOutputFunction charEncodeToArray(std::size_t n, byte* arr);

CharDecodeInputFunction charDecodeFromArray(std::size_t n, const byte* arr);

CharDecodeOutputFunction charDecodeToNull();
CharDecodeOutputFunction charDecodeToStream(u32ostringstream& stream);

extern MultiByteCharacterSet mbcs_utf8;

bool isPrintable(char32_t c);

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
