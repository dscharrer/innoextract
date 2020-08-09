/*
 * Copyright (C) 2011-2020 Daniel Scharrer
 *
 * This software is provided 'as-is', without any express or implied
 * warranty.  In no event will the author(s) be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 */

/*!
 * \file
 *
 * Utility function to convert strings to UTF-8.
 */
#ifndef INNOEXTRACT_UTIL_ENCODING_HPP
#define INNOEXTRACT_UTIL_ENCODING_HPP

#include <bitset>
#include <string>

#include <boost/cstdint.hpp>

namespace util {

enum known_codepages {
	cp_dos708       =   708, // arabic
	cp_windows874   =   874, // thai
	cp_shift_jis    =   932, // japanese
	cp_gbk          =   936, // chinese
	cp_uhc          =   949, // korean
	cp_big5         =   950, // chinese
	cp_big5_hkscs   =   951, // chinese
	cp_utf16le      =  1200,
	cp_utf16be      =  1201,
	cp_windows1250  =  1250, // latin
	cp_windows1251  =  1251, // cyrillic
	cp_windows1252  =  1252, // latin
	cp_windows1253  =  1253, // greek
	cp_windows1254  =  1254, // turkish
	cp_windows1255  =  1255, // hebrew
	cp_windows1256  =  1256, // arabic
	cp_windows1257  =  1257, // baltic
	cp_windows1258  =  1258, // vietnamese
	cp_windows1270  =  1270, // sami
	cp_johab        =  1361, // korean
	cp_macroman     = 10000, // latin
	cp_macjapanese  = 10001, // japanese
	cp_macchinese1  = 10002, // chinese
	cp_mackorean    = 10003, // korean
	cp_macarabic    = 10004, // arabic
	cp_machebrew    = 10005, // hebrew
	cp_macgreek     = 10006, // greek
	cp_maccyrillic  = 10007, // cyrillic
	cp_macchinese2  = 10008, // chinese
	cp_macromania   = 10010, // latin
	cp_macukraine   = 10017, // cyrillic
	cp_macthai      = 10021, // thai
	cp_macroman2    = 10029, // latin
	cp_maciceland   = 10079, // latin
	cp_macturkish   = 10081, // turkish
	cp_maccroatian  = 10082, // latin
	cp_utf32le      = 12000,
	cp_utf32be      = 12001,
	cp_cns          = 20000, // chinese
	cp_big5_eten    = 20002, // chinese
	cp_ia5          = 20105, // latin
	cp_ia5_de       = 20106, // latin
	cp_ia5_se2      = 20107, // latin
	cp_ia5_no2      = 20108, // latin
	cp_ascii        = 20127, // latin
	cp_t61          = 20261, // latin
	cp_iso_6937     = 20269, // latin
	cp_ibm273       = 20273, // latin
	cp_ibm277       = 20277, // latin
	cp_ibm278       = 20278, // latin
	cp_ibm280       = 20280, // latin
	cp_ibm284       = 20284, // latin
	cp_ibm285       = 20285, // latin
	cp_ibm290       = 20290, // japanese
	cp_ibm297       = 20297, // latin
	cp_ibm420       = 20420, // arabic
	cp_ibm423       = 20423, // greek
	cp_ibm424       = 20424, // hebrew
	cp_ibm833       = 20833, // korean
	cp_ibm838       = 20838, // thai
	cp_koi8_r       = 20866, // cyrillic
	cp_ibm871       = 20871, // latin
	cp_ibm880       = 20880, // cyrillic
	cp_ibm905       = 20905, // turkish
	cp_ibm924       = 20924, // latin
	cp_euc_jp_ms    = 20932, // japanese
	cp_gb2312_80    = 20936, // chinese
	cp_wansung      = 20949, // korean
	cp_ibm1025      = 21025, // cyrillic
	cp_koi8_u       = 21866, // cyrillic
	cp_iso_8859_1   = 28591, // latin
	cp_iso_8859_2   = 28592, // latin
	cp_iso_8859_3   = 28593, // latin
	cp_iso_8859_4   = 28594, // latin
	cp_iso_8859_5   = 28595, // cyrillic
	cp_iso_8859_6   = 28596, // arabic
	cp_iso_8859_7   = 28597, // greek
	cp_iso_8859_8   = 28598, // hebrew
	cp_iso_8859_9   = 28599, // turkish
	cp_iso_8859_10  = 28600, // latin
	cp_iso_8859_11  = 28601, // thai
	cp_iso_8859_13  = 28603, // baltic
	cp_iso_8859_14  = 28604, // celtic
	cp_iso_8859_15  = 28605, // latin
	cp_europa3      = 29001, // latin
	cp_iso_8859_6i  = 38596, // hebrew
	cp_iso_8859_8i  = 38598, // hebrew
	cp_iso_2022_jp  = 50220, // japanese
	cp_iso_2022_jp2 = 50221, // japanese
	cp_iso_2022_jp3 = 50222, // japanese
	cp_iso_2022_kr  = 50225, // korean
	cp_iso_2022_cn  = 50227, // chinese
	cp_iso_2022_cn2 = 50229, // chinese
	cp_ibm930       = 50930, // japanese
	cp_ibm931       = 50931, // japanese
	cp_ibm933       = 50933, // korean
	cp_ibm935       = 50935, // chinese
	cp_ibm936       = 50936, // chinese
	cp_ibm937       = 50937, // chinese
	cp_ibm939       = 50939, // japanese
	cp_euc_jp       = 51932, // japanese
	cp_euc_cn       = 51936, // chinese
	cp_euc_kr       = 51949, // korean
	cp_euc_tw       = 51950, // chinese
	cp_gb2312_hz    = 52936, // chinese
	cp_gb18030      = 54936, // chinese
	cp_utf7         = 65000,
	cp_utf8         = 65001,
};

typedef boost::uint32_t codepage_id;

/*!
 * Convert a possibly broken UTF-16 string to WTF-8, an extension of UTF-8.
 */
void utf16le_to_wtf8(const std::string & from, std::string & to);

/*!
 * Find the end of the last complete WTF-8 character in a string.
 */
const char * wtf8_find_end(const char * begin, const char * end);

/*!
 * Convert WTF-8 to UTF-16 while preserving unpaired surrogates.
 */
void wtf8_to_utf16le(const char * begin, const char * end, std::string & to);

/*!
 * Convert WTF-8 to UTF-16 while preserving unpaired surrogates.
 */
void wtf8_to_utf16le(const std::string & from, std::string & to);

/*!
 * Convert a string in place to UTF-8 from a specified encoding.
 * \param data       The input string to convert.
 * \param codepage   The Windows codepage number for the input string encoding.
 * \param lead_bytes Preserve 0x5C path separators.
 *
 * \note This function is not thread-safe.
 */
void to_utf8(std::string & data, codepage_id codepage = cp_windows1252,
             const std::bitset<256> * lead_bytes = NULL);

/*!
 * Convert a string from UTF-8 to a specified encoding.
 * \param from     The input string to convert.
 * \param to       The output for the converted string.
 * \param codepage The Windows codepage number for the input string encoding.
 *
 * \note This function is not thread-safe.
 */
void from_utf8(const std::string & from, std::string & to, codepage_id codepage = cp_windows1252);

std::string encoding_name(codepage_id codepage);

} // namespace util

#endif // INNOEXTRACT_UTIL_ENCODING_HPP
