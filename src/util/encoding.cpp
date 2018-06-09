/*
 * Copyright (C) 2011-2018 Daniel Scharrer
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

enum known_codepages {
	cp_utf16le = 1200,
	cp_windows1252 = 1252,
	cp_ascii = 20127,
	cp_iso_8859_1 = 28591,
	cp_utf8  = 65001,
};

namespace {

//! Get names for encodings where iconv doesn't have the codepage alias
static const char * get_encoding_name(codepage_id codepage) {
	switch(codepage) {
		case   708: return "ISO-8859-6";
		case   936: return "GBK";
		case   949: return "UHC";
		case   950: return "BIG5";
		// iconv's behavior for "UTF-16" is platform-dependent if there is no BOM.
		// There never is any BOM in Inno Setup files and it's always little-endian,
		// so we specify the exact encoding.
		case  1200: return "UTF-16LE";
		case  1201: return "UTF-16BE";
		case  1252: return "MS-ANSI";
		case  1361: return "JOHAB";
		case 10000: return "MACINTOSH";
		case 10002: return "BIG5";
		case 10008: return "GB2312";
		case 12000: return "UTF-32LE";
		case 12001: return "UTF-32BE";
		case 20003: return "IBM5550";
		case 20127: return "US-ASCII";
		case 20261: return "T.61";
		case 20269: return "ISO_6937";
		case 20273: return "IBM273";
		case 20277: return "IBM277";
		case 20278: return "IBM278";
		case 20280: return "IBM280";
		case 20284: return "IBM284";
		case 20285: return "IBM285";
		case 20290: return "IBM290";
		case 20297: return "IBM297";
		case 20420: return "IBM420";
		case 20423: return "IBM423";
		case 20424: return "IBM424";
		case 20866: return "KOI8-R";
		case 20871: return "IBM871";
		case 20880: return "IBM880";
		case 20905: return "IBM905";
		case 20924: return "IBM1047";
		case 20932: return "EUC-JP-MS";
		case 20936: return "EUC-CN";
		case 21025: return "IBM1025";
		case 21866: return "KOI8-U";
		case 28591: return "ISO-8859-1";
		case 28592: return "ISO-8859-2";
		case 28593: return "ISO-8859-3";
		case 28594: return "ISO-8859-4";
		case 28595: return "ISO-8859-5";
		case 28596: return "ISO-8859-6";
		case 28597: return "ISO-8859-7";
		case 28598: return "ISO-8859-8";
		case 28599: return "ISO-8859-9";
		case 28603: return "ISO-8859-13";
		case 28605: return "ISO-8859-15";
		case 38598: return "ISO-8859-8";
		case 50220: return "ISO-2022-JP";
		case 50221: return "ISO-2022-JP-2";
		case 50222: return "ISO-2022-JP-3";
		case 50225: return "ISO-2022-KR";
		case 50227: return "ISO-2022-CN";
		case 50229: return "ISO-2022-CN-EXT";
		case 50930: return "EBCDIC-JP-E";
		case 51932: return "EUC-JP";
		case 51936: return "EUC-CN";
		case 51949: return "EUC-KR";
		case 51950: return "EUC-CN";
		case 54936: return "GB18030";
		case 65000: return "UTF-7";
		case 65001: return "UTF-8";
		default: return NULL;
	}
}

static const char replacement_char = '_';

typedef boost::uint32_t unicode_char;

static size_t get_encoding_size(codepage_id codepage) {
	switch(codepage) {
		case  1200: return 2u; // UTF-16LE
		case  1201: return 2u; // UTF-16BE
		case 12000: return 4u; // UTF-32LE
		case 12001: return 4u; // UTF-32BE
		default:    return 1u;
	}
}

//! Fallback conversion that will at least work for ASCII characters
static void to_utf8_fallback(const std::string & from, std::string & to, codepage_id cp) {
	
	size_t skip = get_encoding_size(cp);
	
	size_t shift = 0;
	switch(cp) {
		case  1201: shift = 1u * 8u; break; // UTF-16BE
		case 12001: shift = 3u * 8u; break; // UTF-32BE
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
			ascii = replacement_char;
		}
		
		to.push_back(ascii);
	}
	
	if(warn) {
		log_warning << "Unknown data while converting from CP" << cp << " to UTF-8.";
	}
	
}

bool is_utf8_continuation_byte(unicode_char chr) {
	return (chr & 0xc0) == 0x80;
}

template <typename In>
unicode_char utf8_read(In & it, In end, unicode_char replacement = unicode_char(replacement_char)) {
	
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

static size_t utf8_length(unicode_char chr) {
	if     (chr <  0x80)       return 1;
	else if(chr <  0x800)      return 2;
	else if(chr <  0x10000)    return 3;
	else if(chr <= 0x0010ffff) return 4;
	return 1;
}

static void utf8_write(std::string & to, unicode_char chr) {
	
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
static bool is_utf16_high_surrogate(unicode_char chr) {
	return chr >= 0xd800 && chr <= 0xdbff;
}

//! \return true c is is the second part of an UTF-16 surrogate pair
static bool is_utf16_low_surrogate(unicode_char chr) {
	return chr >= 0xdc00 && chr <= 0xdfff;
}

static void utf16le_to_utf8(const std::string & from, std::string & to) {
	
	if(from.size() % 2 != 0) {
		log_warning << "Unexpected trailing byte in UTF-16 string.";
	}
	
	to.clear();
	to.reserve(from.size() / 2); // optimistically, most strings only have ASCII characters
	
	bool warn = false;
	
	std::string::const_iterator it = from.begin();
	std::string::const_iterator end = from.end();
	while(it != end) {
		
		unicode_char chr = boost::uint8_t(*it++);
		if(it == end) {
			warn = true;
			utf8_write(to, replacement_char);
			break;
		}
		chr |= unicode_char(boost::uint8_t(*it++)) << 8;
		
		// If it's a surrogate pair, convert to a single UTF-32 character
		if(is_utf16_high_surrogate(chr)) {
			if(it == end) {
				warn = true;
				utf8_write(to, replacement_char);
				break;
			}
			unicode_char d = boost::uint8_t(*it++);
			if(it == end) {
				warn = true;
				utf8_write(to, replacement_char);
				break;
			}
			d |= unicode_char(boost::uint8_t(*it++)) << 8;
			if(is_utf16_low_surrogate(d)) {
				chr = ((chr - 0xd800) << 10) + (d - 0xdc00) + 0x0010000;
			} else {
				warn = true;
				utf8_write(to, replacement_char);
				continue;
			}
		}
		
		// Replace invalid characters
		if(chr > 0x0010FFFF) {
			warn = true;
			// Invalid character (greater than the maximum unicode value)
			utf8_write(to, replacement_char);
			continue;
		}
		
		utf8_write(to, chr);
	}
	
	if(warn) {
		log_warning << "Unexpected data while converting from UTF-16LE to UTF-8.";
	}
	
}

void utf8_to_utf16le(const std::string & from, std::string & to) {
	
	to.clear();
	to.reserve(from.size() * 2); // optimistically, most strings only have ASCII characters
	
	bool warn = false;
	
	for(std::string::const_iterator i = from.begin(); i != from.end(); ) {
		
		unicode_char chr = utf8_read(i, from.end());
		
		if((chr >= 0xd800 && chr <= 0xdfff) || chr > 0x10ffff) {
			chr = replacement_char;
			warn = true;
		} else if(chr >= 0x10000) {
			chr -= 0x10000;
			unicode_char high_surrogate = 0xd800 + (chr >> 10);
			to.push_back(char(boost::uint8_t(high_surrogate)));
			to.push_back(char(boost::uint8_t(high_surrogate >> 8)));
			chr = 0xdc00 + (chr & 0x3ff);
		}
		
		to.push_back(char(boost::uint8_t(chr)));
		to.push_back(char(boost::uint8_t(chr >> 8)));
	}
	
	if(warn) {
		log_warning << "Unexpected data while converting from UTF-8 to UTF-16LE.";
	}
	
}

static unicode_char windows1252_replacements[] = {
	0x20ac, replacement_char, 0x201a, 0x192, 0x201e, 0x2026, 0x2020, 0x2021, 0x2c6,
	0x2030, 0x160, 0x2039, 0x152, replacement_char, 0x17d, replacement_char,
	replacement_char, 0x2018, 0x2019, 0x201c, 0x201d, 0x2022, 0x2013, 0x2014, 0x2dc,
	0x2122, 0x161, 0x203a, 0x153, replacement_char, 0x17e, 0x178
};

BOOST_STATIC_ASSERT(sizeof(windows1252_replacements) == (160 - 128) * sizeof(*windows1252_replacements));

static void windows1252_to_utf8(const std::string & from, std::string & to) {
	
	to.clear();
	to.reserve(from.size()); // optimistically, most strings only have ASCII characters
	
	bool warn = false;
	
	BOOST_FOREACH(char c, from) {
		
		// Windows-1252 maps almost directly to Unicode - yay!
		unicode_char chr = boost::uint8_t(c);
		if(chr >= 128 && chr < 160) {
			chr = windows1252_replacements[chr - 128];
			warn = warn || (chr == unicode_char(replacement_char));
		}
		
		utf8_write(to, chr);
	}
	
	if(warn) {
		log_warning << "Unexpected data while converting from Windows-1252 to UTF-8.";
	}
	
}

static void utf8_to_windows1252(const std::string & from, std::string & to) {
	
	to.clear();
	to.reserve(from.size()); // optimistically, most strings only have ASCII characters
	
	bool warn = false;
	
	for(std::string::const_iterator i = from.begin(); i != from.end(); ) {
		
		unicode_char chr = utf8_read(i, from.end());
		
		// Windows-1252 maps almost directly to Unicode - yay!
		if(chr >= 256 || (chr >= 128 && chr < 160)) {
			size_t i = 0;
			for(; i < boost::size(windows1252_replacements); i++) {
				if(chr == windows1252_replacements[i] && windows1252_replacements[i] != replacement_char) {
					break;
				}
			}
			if(i < boost::size(windows1252_replacements)) {
				chr = unicode_char(128 + i);
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
static converter_map converters;

static iconv_t get_converter(codepage_id codepage) {
	
	// Try to reuse an existing converter if possible
	converter_map::const_iterator i = converters.find(codepage);
	if(i != converters.end()) {
		return i->second;
	}
	
	iconv_t handle = iconv_t(-1);
	
	const char * encoding = get_encoding_name(codepage);
	if(encoding) {
		handle = iconv_open("UTF-8", encoding);
	}
	
	// Otherwise, try a few different codepage name prefixes
	if(handle == iconv_t(-1)) {
		const char * prefixes[] = { "MSCP", "CP", "WINDOWS-", "MS", "IBM", "IBM-", "" };
		BOOST_FOREACH(const char * prefix, prefixes) {
			std::ostringstream oss;
			oss << prefix << std::setfill('0') << std::setw(3) << codepage;
			handle = iconv_open("UTF-8", oss.str().c_str());
			if(handle != iconv_t(-1)) {
				break;
			}
		}
	}
	
	if(handle == iconv_t(-1)) {
		log_warning << "Could not get codepage " << codepage << " -> UTF-8 converter.";
	}
	
	return converters[codepage] = handle;
}

static bool to_utf8_iconv(const std::string & from, std::string & to, codepage_id cp) {
	
	iconv_t converter = get_converter(cp);
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
	
	size_t skip = get_encoding_size(cp);
	
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
					to.push_back(replacement_char);
				} else {
					*outbuf = replacement_char;
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
		log_warning << "Unexpected data while converting from CP" << cp << " to UTF-8.";
	}
	
	to.resize(outbase);
	
	return true;
}

#endif // INNOEXTRACT_HAVE_ICONV

#if INNOEXTRACT_HAVE_WIN32_CONV

static std::string windows_error_string(DWORD code) {
	char * error;
	DWORD n = FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER,
	                         NULL, code, 0, reinterpret_cast<char *>(&error), 0,
	                         NULL);
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

static bool to_utf8_win32(const std::string & from, std::string & to, codepage_id cp) {
	
	int ret = 0;
	
	// Convert from the source codepage to UTF-16LE
	const WCHAR * utf16;
	int utf16_size;
	std::vector<WCHAR> buffer;
	if(cp == cp_utf16le) {
		utf16 = reinterpret_cast<const WCHAR *>(from.data());
		utf16_size = int(from.size()) / 2;
	} else {
		utf16_size = MultiByteToWideChar(cp, 0, from.data(), int(from.length()), NULL, 0);
		if(utf16_size > 0) {
			buffer.resize(size_t(utf16_size));
			ret = MultiByteToWideChar(cp, 0, from.data(), int(from.length()),
			                          &buffer.front(), utf16_size);
		}
		if(utf16_size <= 0 || ret <= 0) {
			log_warning << "Error while converting from CP" << cp << " to UTF-16: "
			            << windows_error_string(GetLastError());
			return false;
		}
		utf16 = &buffer.front();
	}
	
	// Convert from UTF-16LE to UTF-8
	int utf8_size = WideCharToMultiByte(CP_UTF8, 0, utf16, utf16_size, NULL, 0,  NULL, NULL);
	if(utf8_size > 0) {
		to.resize(size_t(utf8_size));
		ret = WideCharToMultiByte(CP_UTF8, 0, utf16, utf16_size,
		                          &to[0], utf8_size, NULL, NULL);
	}
	if(utf8_size <= 0 || ret <= 0) {
		log_warning << "Error while converting from UTF-16 to UTF-8: "
		            << windows_error_string(GetLastError());
		return false;
	}
	
	return true;
}

#endif // INNOEXTRACT_HAVE_WIN32_CONV

} // anonymous namespace

void to_utf8(const std::string & from, std::string & to, codepage_id cp) {
	
	if(from.empty()) {
		to.clear();
		return;
	}
	
	if(cp == cp_utf8 || cp == cp_ascii) {
		to = from;
		return;
	}
	
	switch(cp) {
		case cp_utf16le:     utf16le_to_utf8(from, to); return;
		case cp_windows1252: windows1252_to_utf8(from, to); return;
		case cp_iso_8859_1:  windows1252_to_utf8(from, to); return;
		default: break;
	}
	
	#if INNOEXTRACT_HAVE_ICONV
	if(to_utf8_iconv(from, to, cp)) {
		return;
	}
	#endif
	
	#if INNOEXTRACT_HAVE_WIN32_CONV
	if(to_utf8_win32(from, to, cp)) {
		return;
	}
	#endif
	
	to_utf8_fallback(from, to, cp);
	
}

void from_utf8(const std::string & from, std::string & to, codepage_id codepage) {
	
	if(from.empty()) {
		to.clear();
		return;
	}
	
	if(codepage == cp_utf8) {
		to = from;
		return;
	}
	
	switch(codepage) {
		case cp_utf16le:     utf8_to_utf16le(from, to); return;
		case cp_windows1252: utf8_to_windows1252(from, to); return;
		case cp_iso_8859_1:  utf8_to_windows1252(from, to); return;
		default: {
			log_warning << "Unsupported output codepage: " << codepage;
			to = from;
		}
	}
	
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
