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
// common/hexconv.hh -- header for hex data conversions

#ifndef HEXBED_COMMON_HEXCONV_HH
#define HEXBED_COMMON_HEXCONV_HH

#include <string>
#include <vector>

#include "common/types.hh"

namespace hexbed {

int decDigitToNum(char c);
int hexDigitToNum(char c);
// out must have two chars worth of space
void fastByteConv(char* out, const char* hex, byte b);
void fastByteConvX(strchar* out, const strchar* hex, byte b);

string hexFromBytes(bufsize len, const byte* data, bool upper,
                    bool cont = false);
bool hexToBytes(bufsize& len, byte* data, const string& text);

bool convertBaseFrom(bufsize& out, stringview text, unsigned base);
string convertBaseTo(bufsize in, unsigned base, bool upper);

bool convertBaseFromNeg(bufsize& out, int& neg, stringview text, unsigned base);
string convertBaseToNeg(bufsize in, int neg, unsigned base, bool upper);

};  // namespace hexbed

#endif /* HEXBED_COMMON_HEXCONV_HH */
