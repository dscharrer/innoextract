/*
 * Copyright (C) 2011-2013 Daniel Scharrer
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

#include "util/load.hpp"

#include <iterator>
#include <sstream>
#include <algorithm>
#include <iomanip>

#include <iconv.h>
#include <errno.h>

#include <boost/foreach.hpp>
#include <boost/unordered_map.hpp>

#include "util/log.hpp"

namespace util {

namespace {

static const codepage_id cp_utf8  = 65001;
static const codepage_id cp_ascii = 20127;
static const char replacement_char = '_';

typedef boost::unordered_map<codepage_id, iconv_t> converter_map;
converter_map converters;

static size_t get_encoding_size(codepage_id codepage) {
	switch(codepage) {
		case  1200: return 2u; // UTF-16LE
		case  1201: return 2u; // UTF-16BE
		case 12000: return 4u; // UTF-32LE
		case 12001: return 4u; // UTF-32BE
		default:    return 1u;
	}
}

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
		log_warning << "could not get codepage " << codepage << " -> UTF-8 converter";
	}
	
	return converters[codepage] = handle;
}

//! Fallback conversion that will at least work for ASCII characters
static void to_utf8_fallback(const std::string & from, std::string & to,
                             codepage_id codepage) {
	
	size_t skip = get_encoding_size(codepage);
	
	to.clear();
	to.reserve(ceildiv(from.size(), skip));
	
	for(size_t i = 0; i < from.size(); i += skip) {
		if((unsigned char)from[i] <= 127) {
			// copy ASCII characters
			to.push_back(from[i]);
		} else {
			// replace everything else with underscores
			to.push_back(replacement_char);
		}
	}
}

} // anonymous namespace

void binary_string::load(std::istream & is, std::string & target) {
	
	boost::uint32_t length = util::load<boost::uint32_t>(is);
	if(is.fail()) {
		return;
	}
	
	target.clear();
	
	while(length) {
		char buffer[10 * 1024];
		boost::uint32_t buf_size = std::min(length, boost::uint32_t(sizeof(buffer)));
		is.read(buffer, std::streamsize(buf_size));
		target.append(buffer, buf_size);
		length -= buf_size;
	}
}

void binary_string::skip(std::istream&  is) {
	
	boost::uint32_t length = util::load<boost::uint32_t>(is);
	if(is.fail()) {
		return;
	}
	
	discard(is, length);
}

void encoded_string::load(std::istream & is, std::string & target, codepage_id codepage) {
	to_utf8(binary_string::load(is), target, codepage);
}

void to_utf8(const std::string & from, std::string & to, codepage_id codepage) {
	
	if(codepage == cp_utf8 || codepage == cp_ascii) {
		// copy UTF-8 directly
		to = from;
		return;
	}
	
	iconv_t converter = get_converter(codepage);
	if(converter == iconv_t(-1)) {
		to_utf8_fallback(from, to, codepage);
		return;
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
	
	if(!insize) {
		to.clear();
		return;
	}
	
	iconv(converter, NULL, NULL, NULL, NULL);
	
	size_t skip = get_encoding_size(codepage);
	
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
		log_warning << "unexpected data while converting from CP" << codepage << " to UTF-8";
	}
	
	to.resize(outbase);
}

} // namespace util
