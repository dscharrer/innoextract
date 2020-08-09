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
// Parts based on:
////////////////////////////////////////////////////////////
//
// SFML - Simple and Fast Multimedia Library
// Copyright (C) 2007-2009 Laurent Gomila (laurent.gom@gmail.com)
//
// This software is provided 'as-is', without any express or implied warranty.
// In no event will the authors be held liable for any damages arising from the
// use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it freely,
// subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented;
//    you must not claim that you wrote the original software.
//    If you use this software in a product, an acknowledgment
//    in the product documentation would be appreciated but is not required.
//
// 2. Altered source versions must be plainly marked as such,
//    and must not be misrepresented as being the original software.
//
// 3. This notice may not be removed or altered from any source distribution.
//
////////////////////////////////////////////////////////////
//
// This code has been taken from SFML and altered to fit the project's needs.
//
////////////////////////////////////////////////////////////

#include "util/encoding.hpp"

#include <stddef.h>

#include <algorithm>
#include <iomanip>
#include <iterator>
#include <sstream>
#include <vector>

#include "configure.hpp"

#if INNOEXTRACT_HAVE_ICONV
#include <iconv.h>
#include <errno.h>
#endif

#if INNOEXTRACT_HAVE_WIN32_CONV
#include <windows.h>
#endif

#include <boost/foreach.hpp>
#include <boost/static_assert.hpp>
#include <boost/unordered_map.hpp>
#include <boost/range/size.hpp>

#include "util/log.hpp"
#include "util/math.hpp"

namespace util {

namespace {

//! Get names for encodings where iconv doesn't have the codepage alias
const char * get_encoding_name(codepage_id codepage) {
	switch(codepage) {
		case cp_ascii:        return "US-ASCII";
		case cp_big5:         return "BIG5";
		case cp_big5_eten:    return "BIG5";
		case cp_big5_hkscs:   return "BIG5-HKSCS";
		case cp_cns:          return "EUC-TW";
		case cp_dos708:       return "ISO-8859-6";
		case cp_euc_cn:       return "EUC-CN";
		case cp_euc_jp:       return "EUC-JP";
		case cp_euc_jp_ms:    return "EUC-JP-MS";
		case cp_euc_kr:       return "EUC-KR";
		case cp_euc_tw:       return "EUC-TW";
		case cp_gb2312_80:    return "GB2312";
		case cp_gb2312_hz:    return "GB2312";
		case cp_gb18030:      return "GB18030";
		case cp_gbk:          return "GBK";
		case cp_ia5:          return "ISO_646.IRV:1991";
		case cp_ia5_de:       return "ISO646-DE";
		case cp_ia5_no2:      return "ISO646-NO2";
		case cp_ia5_se2:      return "ISO646-SE2";
		case cp_ibm273:       return "IBM273";
		case cp_ibm277:       return "IBM277";
		case cp_ibm278:       return "IBM278";
		case cp_ibm280:       return "IBM280";
		case cp_ibm284:       return "IBM284";
		case cp_ibm285:       return "IBM285";
		case cp_ibm290:       return "IBM290";
		case cp_ibm297:       return "IBM297";
		case cp_ibm420:       return "IBM420";
		case cp_ibm423:       return "IBM423";
		case cp_ibm424:       return "IBM424";
		case cp_ibm833:       return "IBM833";
		case cp_ibm838:       return "IBM1160";
		case cp_ibm871:       return "IBM871";
		case cp_ibm880:       return "IBM880";
		case cp_ibm905:       return "IBM905";
		case cp_ibm924:       return "IBM1047";
		case cp_ibm930:       return "IBM930";
		case cp_ibm931:       return "IBM931";
		case cp_ibm933:       return "IBM933";
		case cp_ibm935:       return "IBM935";
		case cp_ibm936:       return "IBM936";
		case cp_ibm937:       return "IBM937";
		case cp_ibm939:       return "IBM939";
		case cp_ibm1025:      return "IBM1025";
		case cp_iso_2022_cn:  return "ISO-2022-CN";
		case cp_iso_2022_cn2: return "ISO-2022-CN-EXT";
		case cp_iso_2022_jp:  return "ISO-2022-JP";
		case cp_iso_2022_jp2: return "ISO-2022-JP-2";
		case cp_iso_2022_jp3: return "ISO-2022-JP-3";
		case cp_iso_2022_kr:  return "ISO-2022-KR";
		case cp_iso_6937:     return "ISO_6937";
		case cp_iso_8859_10:  return "ISO-8859-10";
		case cp_iso_8859_11:  return "ISO-8859-11";
		case cp_iso_8859_13:  return "ISO-8859-13";
		case cp_iso_8859_14:  return "ISO-8859-14";
		case cp_iso_8859_15:  return "ISO-8859-15";
		case cp_iso_8859_1:   return "ISO-8859-1";
		case cp_iso_8859_2:   return "ISO-8859-2";
		case cp_iso_8859_3:   return "ISO-8859-3";
		case cp_iso_8859_4:   return "ISO-8859-4";
		case cp_iso_8859_5:   return "ISO-8859-5";
		case cp_iso_8859_6:   return "ISO-8859-6";
		case cp_iso_8859_6i:  return "ISO-8859-6";
		case cp_iso_8859_7:   return "ISO-8859-7";
		case cp_iso_8859_8:   return "ISO-8859-8";
		case cp_iso_8859_8i:  return "ISO-8859-8";
		case cp_iso_8859_9:   return "ISO-8859-9";
		case cp_johab:        return "JOHAB";
		case cp_koi8_r:       return "KOI8-R";
		case cp_koi8_u:       return "KOI8-U";
		case cp_macarabic:    return "MACARABIC";
		case cp_macchinese1:  return "BIG5";
		case cp_macchinese2:  return "EUC-CN";
		case cp_maccroatian:  return "MACCROATIAN";
		case cp_maccyrillic:  return "MACCYRILLIC";
		case cp_macgreek:     return "MACGREEK";
		case cp_machebrew:    return "MACHEBREW";
		case cp_maciceland:   return "MACICELAND";
		case cp_macjapanese:  return "SHIFT-JIS";
		case cp_mackorean:    return "EUC-KR";
		case cp_macroman2:    return "MACCENTRALEUROPE";
		case cp_macroman:     return "MACINTOSH";
		case cp_macromania:   return "MACROMANIA";
		case cp_macthai:      return "MACTHAI";
		case cp_macturkish:   return "MACTURKISH";
		case cp_macukraine:   return "MACUKRAINE";
		case cp_shift_jis:    return "SHIFT-JIS";
		case cp_t61:          return "T.61";
		case cp_uhc:          return "UHC";
		case cp_utf7:         return "UTF-7";
		case cp_utf8:         return "UTF-8";
		case cp_utf16be:      return "UTF-16BE";
		case cp_utf16le:      return "UTF-16LE"; // "UTF-16" is platform-dependent without a BOM
		case cp_utf32be:      return "UTF-32BE";
		case cp_utf32le:      return "UTF-32LE";
		case cp_wansung:      return "EUC-KR";
		case cp_windows1250:  return "MS-EE";
		case cp_windows1251:  return "MS-CYRL";
		case cp_windows1252:  return "MS-ANSI";
		case cp_windows1253:  return "MS-GREEK";
		case cp_windows1254:  return "MS-TURK";
		case cp_windows1255:  return "MS-HEBR";
		case cp_windows1256:  return "MS-ARAB";
		default: return NULL;
	}
}

//! Check if a codepage is known to be a superset of ASCII - used for optimization only
bool is_extended_ascii(codepage_id codepage) {
	
	// cp_utf8 is handled separately
	
	if(codepage >= cp_windows1250 && codepage <= cp_windows1270) {
		return true;
	}
	
	if(codepage >= cp_iso_8859_1 && codepage <= cp_iso_8859_15) {
		return true;
	}
	
	switch(codepage) {
		case cp_ascii:
		case cp_big5:
		case cp_big5_eten:
		case cp_big5_hkscs:
		case cp_cns:
		case cp_dos708:
		case cp_euc_cn:
		case cp_euc_tw:
		case cp_gb18030:
		case cp_gbk:
		case cp_iso_6937:
		case cp_iso_8859_6i:
		case cp_iso_8859_8i:
		case cp_koi8_r:
		case cp_koi8_u:
		case cp_macarabic:
		case cp_macchinese1:
		case cp_macchinese2:
		case cp_maccyrillic:
		case cp_macgreek:
		case cp_maciceland:
		case cp_macroman:
		case cp_uhc:
		case cp_windows874:
			return true;
		default:
			return false;
	}
	
}

bool is_ascii(const std::string & data) {
	// String in an extended ASCII encoding contains only ASCII characters
	BOOST_FOREACH(char c, data) {
		if(boost::uint8_t(c) >= 128) {
			return false;
		}
	}
	return true;
}

//! Check if a string is compatible with UTF-8
bool is_utf8(const std::string & data, codepage_id codepage) {
	
	if(codepage == cp_utf8 || codepage == cp_ascii) {
		return true;
	}
	
	if(is_extended_ascii(codepage) && is_ascii(data)) {
		return true;
	}
	
	return false;
}

typedef boost::uint32_t unicode_char;

const unicode_char replacement_char = '_';

size_t get_code_unit_size(codepage_id codepage) {
	switch(codepage) {
		case cp_utf16le: return 2u;
		case cp_utf16be: return 2u;
		case cp_utf32le: return 4u;
		case cp_utf32be: return 4u;
		default:    return 1u;
	}
}

//! Fallback conversion that will at least work for ASCII characters
void to_utf8_fallback(const std::string & from, std::string & to, codepage_id codepage) {
	
	size_t skip = get_code_unit_size(codepage);
	
	size_t shift = 0;
	switch(codepage) {
		case cp_utf16be: shift = 1u * 8u; break;
		case cp_utf32be: shift = 3u * 8u; break;
		default: break;
	}
	
	to.clear();
	to.reserve(ceildiv(from.size(), skip));
	
	bool warn = false;
	
	for(std::string::const_iterator it = from.begin(); it != from.end();) {
		
		unicode_char unicode = 0;
		for(size_t i = 0; i < skip; i++) {
			unicode |= unicode_char(boost::uint8_t(*it++)) << (i * 8);
		}
		
		char ascii = char((unicode >> shift) & 0x7f);
		
		// replace non-ASCII characters with underscores
		if((unicode_char(ascii) << shift) != unicode) {
			warn = true;
			ascii = char(replacement_char);
		}
		
		to.push_back(ascii);
	}
	
	if(warn) {
		log_warning << "Unknown data while converting from CP" << codepage << " to UTF-8.";
	}
	
}

bool is_utf8_continuation_byte(unicode_char chr) {
	return (chr & 0xc0) == 0x80;
}

template <typename In>
unicode_char utf8_read(In & it, In end, unicode_char replacement = replacement_char) {
	
	if(it == end) {
		return unicode_char(-1);
	}
	unicode_char chr = boost::uint8_t(*it++);
	
	// For multi-byte characters, read the remaining bytes
	if(chr & (1 << 7)) {
		
		if(is_utf8_continuation_byte(chr)) {
			// Bad start position
			return replacement;
		}
		
		if(it == end || !is_utf8_continuation_byte(boost::uint8_t(*it))) {
			// Unexpected end of multi-byte sequence
			return replacement;
		}
		chr &= 0x3f, chr <<= 6, chr |= unicode_char(boost::uint8_t(*it++) & 0x3f);
		
		if(chr & (1 << (5 + 6))) {
			
			if(it == end || !is_utf8_continuation_byte(boost::uint8_t(*it))) {
				// Unexpected end of multi-byte sequence
				return replacement;
			}
			chr &= ~unicode_char(1 << (5 + 6)), chr <<= 6, chr |= unicode_char(boost::uint8_t(*it++) & 0x3f);
			
			if(chr & (1 << (4 + 6 + 6))) {
				
				if(it == end || !is_utf8_continuation_byte(boost::uint8_t(*it))) {
					// Unexpected end of multi-byte sequence
					return replacement;
				}
				chr &= ~unicode_char(1 << (4 + 6 + 6)), chr <<= 6, chr |= unicode_char(boost::uint8_t(*it++) & 0x3f);
				
				if(chr & (1 << (3 + 6 + 6 + 6))) {
					// Illegal UTF-8 byte
					return replacement;
				}
				
			}
		}
	}
	
	return chr;
}

size_t utf8_length(unicode_char chr) {
	if(chr < 0x80) {
		return 1;
	} else if(chr < 0x800) {
		return 2;
	} else if(chr < 0x10000) {
		return 3;
	} else if(chr <= 0x0010ffff) {
		return 4;
	}
	return 1;
}

void utf8_write(std::string & to, unicode_char chr) {
	
	static const boost::uint8_t first_bytes[7] = {
		0x00, 0x00, 0xc0, 0xe0, 0xf0, 0xf8, 0xfc
	};
	
	// Get number of bytes to write
	size_t length = utf8_length(chr);
	
	// Extract bytes to write
	boost::uint8_t bytes[4];
	switch(length) {
		case 4: bytes[3] = static_cast<boost::uint8_t>((chr | 0x80) & 0xBF), chr >>= 6; /* fall-through */
		case 3: bytes[2] = static_cast<boost::uint8_t>((chr | 0x80) & 0xBF), chr >>= 6; /* fall-through */
		case 2: bytes[1] = static_cast<boost::uint8_t>((chr | 0x80) & 0xBF), chr >>= 6; /* fall-through */
		case 1: bytes[0] = static_cast<boost::uint8_t>(chr | first_bytes[length]);
		default: break;
	}
	
	// Add them to the output
	const boost::uint8_t * cur_byte = bytes;
	switch(length) {
		case 4: to.push_back(char(*cur_byte++)); /* fall-through */
		case 3: to.push_back(char(*cur_byte++)); /* fall-through */
		case 2: to.push_back(char(*cur_byte++)); /* fall-through */
		case 1: to.push_back(char(*cur_byte++));
		default: break;
	}
	
}

//! \return true c is is the first part of an UTF-16 surrogate pair
bool is_utf16_high_surrogate(unicode_char chr) {
	return chr >= 0xd800 && chr <= 0xdbff;
}

//! \return true c is is the second part of an UTF-16 surrogate pair
bool is_utf16_low_surrogate(unicode_char chr) {
	return chr >= 0xdc00 && chr <= 0xdfff;
}

} // anonymous namespace

void utf16le_to_wtf8(const std::string & from, std::string & to) {
	
	if(from.size() % 2 != 0) {
		log_warning << "Unexpected trailing byte in UTF-16 string.";
	}
	
	to.clear();
	to.reserve(from.size() / 2); // optimistically, most strings only have ASCII characters
	
	bool warn = false;
	
	std::string::const_iterator it = from.begin();
	std::string::const_iterator end = from.end();
	if(from.size() % 2 != 0) {
		--end;
	}
	while(it != end) {
		
		unicode_char chr = boost::uint8_t(*it++);
		chr |= unicode_char(boost::uint8_t(*it++)) << 8;
		
		// If it's a surrogate pair, convert to a single UTF-32 character
		if(is_utf16_high_surrogate(chr) && it != end) {
			unicode_char d = boost::uint8_t(*it);
			d |= unicode_char(boost::uint8_t(*(it + 1))) << 8;
			if(is_utf16_low_surrogate(d)) {
				chr = ((chr - 0xd800) << 10) + (d - 0xdc00) + 0x0010000;
				it += 2;
			}
		}
		
		utf8_write(to, chr);
	}
	if(end != from.end()) {
		warn = true;
		utf8_write(to, replacement_char);
	}
	
	if(warn) {
		log_warning << "Unexpected data while converting from UTF-16LE to UTF-8.";
	}
	
}

const char * wtf8_find_end(const char * begin, const char * end) {
	
	const char * i = end;
	while(i != begin && is_utf8_continuation_byte(boost::uint8_t(*(i - 1)))) {
		i--;
	}
	
	if(i != begin) {
		unicode_char chr = boost::uint8_t(*(i - 1));
		size_t expected = 0;
		if(chr & (1 << 7)) {
			expected++;
			if(chr & (1 << (5 + 6))) {
				expected++;
				if(chr & (1 << (4 + 6 + 6))) {
					expected++;
				}
			}
		}
		if(expected > size_t(end - i)) {
			return i - 1;
		}
	}
	
	return end;
}

void wtf8_to_utf16le(const char * begin, const char * end, std::string & to) {
	
	to.clear();
	to.reserve(size_t(end - begin) * 2); // optimistically, most strings only have ASCII characters
	
	for(const char * i = begin; i != end; ) {
		
		unicode_char chr = utf8_read(i, end);
		
		if(chr >= 0x10000) {
			chr -= 0x10000;
			unicode_char high_surrogate = 0xd800 + (chr >> 10);
			to.push_back(char(boost::uint8_t(high_surrogate)));
			to.push_back(char(boost::uint8_t(high_surrogate >> 8)));
			chr = 0xdc00 + (chr & 0x3ff);
		}
		
		to.push_back(char(boost::uint8_t(chr)));
		to.push_back(char(boost::uint8_t(chr >> 8)));
	}
	
}

void wtf8_to_utf16le(const std::string & from, std::string & to) {
	return wtf8_to_utf16le(from.c_str(), from.c_str() + from.size(), to);
}

namespace {

unicode_char windows1252_replacements[] = {
	0x20ac, replacement_char, 0x201a, 0x192, 0x201e, 0x2026, 0x2020, 0x2021, 0x2c6,
	0x2030, 0x160, 0x2039, 0x152, replacement_char, 0x17d, replacement_char,
	replacement_char, 0x2018, 0x2019, 0x201c, 0x201d, 0x2022, 0x2013, 0x2014, 0x2dc,
	0x2122, 0x161, 0x203a, 0x153, replacement_char, 0x17e, 0x178
};

BOOST_STATIC_ASSERT(sizeof(windows1252_replacements) == (160 - 128) * sizeof(*windows1252_replacements));

void windows1252_to_utf8(const std::string & from, std::string & to) {
	
	to.clear();
	to.reserve(from.size()); // optimistically, most strings only have ASCII characters
	
	bool warn = false;
	
	BOOST_FOREACH(char c, from) {
		
		// Windows-1252 maps almost directly to Unicode - yay!
		unicode_char chr = boost::uint8_t(c);
		if(chr >= 128 && chr < 160) {
			chr = windows1252_replacements[chr - 128];
			warn = warn || chr == replacement_char;
		}
		
		utf8_write(to, chr);
	}
	
	if(warn) {
		log_warning << "Unexpected data while converting from Windows-1252 to UTF-8.";
	}
	
}

void utf8_to_windows1252(const std::string & from, std::string & to) {
	
	to.clear();
	to.reserve(from.size()); // optimistically, most strings only have ASCII characters
	
	bool warn = false;
	
	for(std::string::const_iterator i = from.begin(); i != from.end(); ) {
		
		unicode_char chr = utf8_read(i, from.end());
		
		// Windows-1252 maps almost directly to Unicode - yay!
		if(chr >= 256 || (chr >= 128 && chr < 160)) {
			size_t j = 0;
			for(; j < size_t(boost::size(windows1252_replacements)); j++) {
				if(chr == windows1252_replacements[j] && windows1252_replacements[j] != replacement_char) {
					break;
				}
			}
			if(j < size_t(boost::size(windows1252_replacements))) {
				chr = unicode_char(128 + j);
			} else {
				chr = replacement_char;
				warn = true;
			}
		}
		
		to.push_back(char(boost::uint8_t(chr)));
	}
	
	if(warn) {
		log_warning << "Unsupported character while converting from UTF-8 to Windows-1252.";
	}
	
}

#if INNOEXTRACT_HAVE_ICONV

typedef boost::unordered_map<codepage_id, iconv_t> converter_map;
converter_map converters;

iconv_t get_converter(codepage_id codepage, bool reverse) {
	
	boost::uint32_t key = codepage | (reverse ? 0x80000000 : 0);
	
	// Try to reuse an existing converter if possible
	converter_map::const_iterator i = converters.find(key);
	if(i != converters.end()) {
		return i->second;
	}
	
	iconv_t handle = iconv_t(-1);
	
	const char * encoding = get_encoding_name(codepage);
	if(encoding) {
		handle = reverse ? iconv_open(encoding, "UTF-8") : iconv_open("UTF-8", encoding);
	}
	
	// Otherwise, try a few different codepage name prefixes
	if(handle == iconv_t(-1)) {
		const char * prefixes[] = { "MSCP", "CP", "WINDOWS-", "MS", "IBM", "IBM-", "" };
		BOOST_FOREACH(const char * prefix, prefixes) {
			std::ostringstream oss;
			oss << prefix << std::setfill('0') << std::setw(3) << codepage;
			handle = reverse ? iconv_open(oss.str().c_str(), "UTF-8") : iconv_open("UTF-8", oss.str().c_str());
			if(handle != iconv_t(-1)) {
				break;
			}
		}
	}
	
	if(handle == iconv_t(-1)) {
		log_warning << "Could not get codepage " << codepage << " -> UTF-8 converter.";
	}
	
	return converters[key] = handle;
}

bool utf8_iconv(const std::string & from, std::string & to, codepage_id codepage, bool reverse) {
	
	iconv_t converter = get_converter(codepage, reverse);
	if(converter == iconv_t(-1)) {
		return false;
	}
	
	/*
	 * Some iconv implementations declare the second parameter of iconv() as
	 * const char **, others as char **.
	 * Use this little hack to compile with both variants.
	 */
	struct inbuf_ {
		const char * buf;
		explicit inbuf_(const char * data) : buf(data) { }
		operator const char **() { return &buf; }
		operator char **() { return const_cast<char **>(&buf); }
	} inbuf(from.data());
	
	size_t insize = from.size();
	
	size_t outbase = 0;
	
	iconv(converter, NULL, NULL, NULL, NULL);
	
	size_t skip = get_code_unit_size(codepage);
	
	bool warn = false;
	
	while(insize) {
		
		to.resize(outbase + ceildiv(insize, skip) + 4);
		
		char * outbuf = &to[0] + outbase;
		size_t outsize = to.size() - outbase;
		
		size_t ret = iconv(converter, inbuf, &insize, &outbuf, &outsize);
		if(ret == size_t(-1)) {
			if(errno == E2BIG) {
				// not enough output space - we'll allocate more in the next loop
			} else if(/*errno == EILSEQ &&*/ insize >= 2) {
				// invalid byte (sequence) - add a replacement char and try the next byte
				if(outsize == 0) {
					to.push_back(char(replacement_char));
				} else {
					*outbuf = char(replacement_char);
					outsize--;
				}
				inbuf.buf += skip;
				insize -= skip;
				warn = true;
			} else {
				// something else went wrong - return what we have so far
				insize = 0;
				warn = true;
			}
		}
		
		outbase = to.size() - outsize;
	}
	
	if(warn) {
		if(reverse) {
			log_warning << "Unexpected data while converting from UTF-8 to CP" << codepage << ".";
		} else {
			log_warning << "Unexpected data while converting from CP" << codepage << " to UTF-8.";
		}
	}
	
	to.resize(outbase);
	
	return true;
}

bool to_utf8_iconv(const std::string & from, std::string & to, codepage_id codepage) {
	return utf8_iconv(from, to, codepage, false);
}

bool from_utf8_iconv(const std::string & from, std::string & to, codepage_id codepage) {
	return utf8_iconv(from, to, codepage, true);
}

#endif // INNOEXTRACT_HAVE_ICONV

#if INNOEXTRACT_HAVE_WIN32_CONV

std::string windows_error_string(DWORD code) {
	char * error;
	DWORD n = FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER,
	                         NULL, code, 0, reinterpret_cast<char *>(&error), 0, NULL);
	if(n == 0) {
		return "unknown";
	} else {
		std::string ret(error, size_t(n));
		LocalFree(error);
		if(!ret.empty() && ret[ret.size() - 1] == '\n') {
			ret.resize(ret.size() - 1);
		}
		return ret;
	}
}

bool to_utf8_win32(const std::string & from, std::string & to, codepage_id codepage) {
	 
	// Convert from the source codepage to UTF-16LE
	std::string buffer;
	int ret = MultiByteToWideChar(codepage, 0, from.data(), int(from.length()), NULL, 0);
	if(ret > 0) {
		buffer.resize(size_t(ret) * 2);
		ret = MultiByteToWideChar(codepage, 0, from.data(), int(from.length()),
		                          reinterpret_cast<LPWSTR>(&buffer[0]), ret);
	}
	if(ret <= 0) {
		log_warning << "Error while converting from CP" << codepage << " to UTF-16: "
		            << windows_error_string(GetLastError());
		return false;
	}
	
	utf16le_to_wtf8(buffer, to);
	
	return true;
}

bool from_utf8_win32(const std::string & from, std::string & to, codepage_id codepage) {
	
	std::string buffer;
	wtf8_to_utf16le(from, buffer);
	
	// Convert from UTF-16LE to the target codepage
	LPCWSTR data = reinterpret_cast<LPCWSTR>(buffer.c_str());
	int size = int(buffer.size() / 2);
	int ret = WideCharToMultiByte(codepage, 0, data, size, NULL, 0,  NULL, NULL);
	if(ret > 0) {
		to.resize(size_t(ret));
		ret = WideCharToMultiByte(codepage, 0, data, size, &to[0], ret, NULL, NULL);
	}
	if(ret <= 0) {
		log_warning << "Error while converting from UTF-16 to CP" << codepage << ": "
		            << windows_error_string(GetLastError());
		return false;
	}
	
	return true;
}

#endif // INNOEXTRACT_HAVE_WIN32_CONV

void to_utf8(const std::string & from, std::string & to, codepage_id codepage,
             const std::bitset<256> * lead_bytes) {
	
	switch(codepage) {
		case cp_utf16le:     utf16le_to_wtf8(from, to); return;
		case cp_windows1252: windows1252_to_utf8(from, to); return;
		case cp_iso_8859_1:  windows1252_to_utf8(from, to); return;
		default: break;
	}
	
	if(lead_bytes) {
		std::string buffer;
		for(size_t start = 0; start < from.length();) {
			size_t end = start;
			while(end < from.length()) {
				if(lead_bytes->test(static_cast<unsigned char>(from[end]))) {
					end = std::min(from.length(), end + 2);
				} else if(from[end] != 0x5C) {
					end++;
				} else {
					break;
				}
			}
			buffer = from.substr(start, end - start);
			util::to_utf8(buffer, codepage, NULL);
			to.append(buffer);
			if(end < from.length()) {
				to.push_back('\\');
			}
			start = end + 1;
		}
		return;
	}
	
	#if INNOEXTRACT_HAVE_ICONV
	if(to_utf8_iconv(from, to, codepage)) {
		return;
	}
	#endif
	
	#if INNOEXTRACT_HAVE_WIN32_CONV
	if(to_utf8_win32(from, to, codepage)) {
		return;
	}
	#endif
	
	to_utf8_fallback(from, to, codepage);
	
}

} // anonymous namespace

void to_utf8(std::string & data, codepage_id codepage, const std::bitset<256> * lead_bytes) {
	
	if(data.empty() || is_utf8(data, codepage)) {
		// Already UTF-8
		return;
	}
	
	std::string buffer;
	to_utf8(data, buffer, codepage, lead_bytes);
	std::swap(data, buffer);
}

void from_utf8(const std::string & from, std::string & to, codepage_id codepage) {
	
	if(from.empty()) {
		to.clear();
		return;
	}
	
	if(codepage == cp_utf8 || (is_extended_ascii(codepage) && is_ascii(from))) {
		to = from;
		return;
	}
	
	switch(codepage) {
		case cp_utf16le:     wtf8_to_utf16le(from, to); return;
		case cp_windows1252: utf8_to_windows1252(from, to); return;
		default: break;
	}
	
	#if INNOEXTRACT_HAVE_ICONV
	if(from_utf8_iconv(from, to, codepage)) {
		return;
	}
	#endif
	
	#if INNOEXTRACT_HAVE_WIN32_CONV
	if(from_utf8_win32(from, to, codepage)) {
		return;
	}
	#endif
	
	log_warning << "Unsupported output codepage: " << codepage;
	to = from;
	
}

std::string encoding_name(codepage_id codepage) {
	
	const char * name = get_encoding_name(codepage);
	if(name) {
		return name;
	}
	
	std::ostringstream oss;
	oss << "Windows-" << codepage;
	
	return oss.str();
}

} // namespace util
