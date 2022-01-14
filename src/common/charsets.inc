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
// common/charsets.inc -- character sets (included by charconv.cc)

// list of charsets
#define CHARSETS_EXTRA_INCLUDE()                                               \
    CHARSETS_EXTRA_INCLUDE_SET(cp437)                                          \
    CHARSETS_EXTRA_INCLUDE_SET(cp850)                                          \

#define U(x) 0x##x##UL

#define ASCII_TABLE_A()                                                        \
    U(0000), U(0001), U(0002), U(0003), U(0004), U(0005), U(0006), U(0007),    \
    U(0008), U(0009), U(000A), U(000B), U(000C), U(000D), U(000E), U(000F),    \
    U(0010), U(0011), U(0012), U(0013), U(0014), U(0015), U(0016), U(0017),    \
    U(0018), U(0019), U(001A), U(001B), U(001C), U(001D), U(001E), U(001F),    \
    U(0020), U(0021), U(0022), U(0023), U(0024), U(0025), U(0026), U(0027),    \
    U(0028), U(0029), U(002A), U(002B), U(002C), U(002D), U(002E), U(002F),    \
    U(0030), U(0031), U(0032), U(0033), U(0034), U(0035), U(0036), U(0037),    \
    U(0038), U(0039), U(003A), U(003B), U(003C), U(003D), U(003E), U(003F),    \
    U(0040), U(0041), U(0042), U(0043), U(0044), U(0045), U(0046), U(0047),    \
    U(0048), U(0049), U(004A), U(004B), U(004C), U(004D), U(004E), U(004F),    \
    U(0050), U(0051), U(0052), U(0053), U(0054), U(0055), U(0056), U(0057),    \
    U(0058), U(0059), U(005A), U(005B), U(005C), U(005D), U(005E), U(005F),    \
    U(0060), U(0061), U(0062), U(0063), U(0064), U(0065), U(0066), U(0067),    \
    U(0068), U(0069), U(006A), U(006B), U(006C), U(006D), U(006E), U(006F),    \
    U(0070), U(0071), U(0072), U(0073), U(0074), U(0075), U(0076), U(0077),    \
    U(0078), U(0079), U(007A), U(007B), U(007C), U(007D), U(007E), U(007F)    

#define ASCII_TABLE_AX()                                                       \
    U(0020), U(0021), U(0022), U(0023), U(0024), U(0025), U(0026), U(0027),    \
    U(0028), U(0029), U(002A), U(002B), U(002C), U(002D), U(002E), U(002F),    \
    U(0030), U(0031), U(0032), U(0033), U(0034), U(0035), U(0036), U(0037),    \
    U(0038), U(0039), U(003A), U(003B), U(003C), U(003D), U(003E), U(003F),    \
    U(0040), U(0041), U(0042), U(0043), U(0044), U(0045), U(0046), U(0047),    \
    U(0048), U(0049), U(004A), U(004B), U(004C), U(004D), U(004E), U(004F),    \
    U(0050), U(0051), U(0052), U(0053), U(0054), U(0055), U(0056), U(0057),    \
    U(0058), U(0059), U(005A), U(005B), U(005C), U(005D), U(005E), U(005F),    \
    U(0060), U(0061), U(0062), U(0063), U(0064), U(0065), U(0066), U(0067),    \
    U(0068), U(0069), U(006A), U(006B), U(006C), U(006D), U(006E), U(006F),    \
    U(0070), U(0071), U(0072), U(0073), U(0074), U(0075), U(0076), U(0077),    \
    U(0078), U(0079), U(007A), U(007B), U(007C), U(007D), U(007E)              

#define CP437_TABLE_A()                                                        \
    U(0000), U(263A), U(263B), U(2665), U(2666), U(2663), U(2660), U(2022),    \
    U(25D8), U(25CB), U(25D9), U(2642), U(2640), U(266A), U(266B), U(263C),    \
    U(25BA), U(25C4), U(2195), U(203C), U(00B6), U(00A7), U(25AC), U(21A8),    \
    U(2191), U(2193), U(2192), U(2190), U(221F), U(2194), U(25B2), U(25BC),    \
    ASCII_TABLE_AX(),                                              U(2302)

// cp437 (IBM 437)
CHARSET_DEFINE_A(cp437) {
    CP437_TABLE_A(),
    U(00C7), U(00FC), U(00E9), U(00E2), U(00E4), U(00E0), U(00E5), U(00E7),//80
    U(00EA), U(00EB), U(00E8), U(00EF), U(00EE), U(00EC), U(00C4), U(00C5),
    U(00C9), U(00E6), U(00C6), U(00F4), U(00F6), U(00F2), U(00FB), U(00F9),//90
    U(00FF), U(00D6), U(00DC), U(00A2), U(00A3), U(00A5), U(20A7), U(0192),
    U(00E1), U(00ED), U(00F3), U(00FA), U(00F1), U(00D1), U(00AA), U(00BA),//A0
    U(00BF), U(2310), U(00AC), U(00BD), U(00BC), U(00A1), U(00AB), U(00BB),
    U(2591), U(2592), U(2593), U(2502), U(2524), U(2561), U(2562), U(2556),//B0
    U(2555), U(2563), U(2551), U(2557), U(255D), U(255C), U(255B), U(2510),
    U(2514), U(2534), U(252C), U(251C), U(2500), U(253C), U(255E), U(255F),//C0
    U(255A), U(2554), U(2569), U(2566), U(2560), U(2550), U(256C), U(2567),
    U(2568), U(2564), U(2565), U(2559), U(2558), U(2552), U(2553), U(256B),//D0
    U(256A), U(2518), U(250C), U(2588), U(2584), U(258C), U(2590), U(2580),
    U(03B1), U(00DF), U(0393), U(03C0), U(03A3), U(03C3), U(00B5), U(03C4),//E0
    U(03A6), U(0398), U(03A9), U(03B4), U(221E), U(03C6), U(03B5), U(2229),
    U(2261), U(00B1), U(2265), U(2264), U(2320), U(2321), U(00F7), U(2248),//F0
    U(00B0), U(2219), U(00B7), U(221A), U(207F), U(00B2), U(25A0), U(00A0)
};

// cp850 (IBM 850)
CHARSET_DEFINE_A(cp850) {
    CP437_TABLE_A(),
    U(00C7), U(00FC), U(00E9), U(00E2), U(00E4), U(00E0), U(00E5), U(00E7),//80
    U(00EA), U(00EB), U(00E8), U(00EF), U(00EE), U(00EC), U(00C4), U(00C5),
    U(00C9), U(00E6), U(00C6), U(00F4), U(00F6), U(00F2), U(00FB), U(00F9),//90
    U(00FF), U(00D6), U(00DC), U(00F8), U(00A3), U(00D8), U(00D7), U(0192),
    U(00E1), U(00ED), U(00F3), U(00FA), U(00F1), U(00D1), U(00AA), U(00BA),//A0
    U(00BF), U(00AE), U(00AC), U(00BD), U(00BC), U(00A1), U(00AB), U(00BB),
    U(2591), U(2592), U(2593), U(2502), U(2524), U(00C1), U(00C2), U(00C0),//B0
    U(00A9), U(2563), U(2551), U(2557), U(255D), U(00A2), U(00A5), U(2510),
    U(2514), U(2534), U(252C), U(251C), U(2500), U(253C), U(00E3), U(00C3),//C0
    U(255A), U(2554), U(2569), U(2566), U(2560), U(2550), U(256C), U(00A4),
    U(00F0), U(00D0), U(00CA), U(00CB), U(00C8), U(0131), U(00CD), U(00CE),//D0
    U(00CF), U(2518), U(250C), U(2588), U(2584), U(00A6), U(00CC), U(2580),
    U(00D3), U(00DF), U(00D4), U(00D2), U(00F5), U(00D5), U(00B5), U(00FE),//E0
    U(00DE), U(00DA), U(00DB), U(00D9), U(00FD), U(00DD), U(00AF), U(00B4),
    U(00AD), U(00B1), U(2017), U(00BE), U(00B6), U(00A7), U(00F7), U(00B8),//F0
    U(00B0), U(00A8), U(00B7), U(00B9), U(00B3), U(00B2), U(25A0), U(00A0)
};
