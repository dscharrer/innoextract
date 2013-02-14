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
#include <map>
#include <sstream>
#include <algorithm>

#include <iconv.h>
#include <errno.h>

#include "util/log.hpp"

namespace {

static const boost::uint32_t cp_utf8 = 65001;
static const boost::uint32_t cp_utf16 = 1200;
static const char replacement_char = '_';

std::map<boost::uint32_t, iconv_t> converters;

iconv_t get_converter(boost::uint32_t codepage) {
	
	std::map<boost::uint32_t, iconv_t>::iterator i = converters.find(codepage);
	
	if(i != converters.end()) {
		return i->second;
	}
	
	std::ostringstream oss;
	if(codepage == cp_utf16) {
		// iconv's behavior for "UTF-16" is platform-dependant if there is no BOM.
		// There never is any BOM in Inno Setup files and it's always little-endian,
		// so we specify the exact encoding.
		oss << "UTF-16LE";
	} else {
		oss << "CP" << codepage;
	}
	
	iconv_t handle = iconv_open("UTF-8", oss.str().c_str());
	
	if(handle == iconv_t(-1)) {
		log_warning << "could not get " << oss.str() << " -> UTF-8 converter";
	}
	
	return converters[codepage] = handle;
}

};

void binary_string::load(std::istream & is, std::string & target) {
	
	boost::int32_t length = load_number<boost::int32_t>(is);
	if(is.fail()) {
		return;
	}
	
	target.clear();
	
	while(length) {
		char buffer[10 * 1024];
		boost::int32_t buf_size = std::min(length, boost::int32_t(sizeof(buffer)));
		is.read(buffer, buf_size);
		target.append(buffer, size_t(buf_size));
		length -= buf_size;
	}
}

void encoded_string::load(std::istream & is, std::string & target, boost::uint32_t codepage) {
	
	std::string temp;
	binary_string::load(is, temp);
	
	to_utf8(temp, target, codepage);
}

//! Fallback conversion that will at least work for ASCII characters
static void to_utf8_fallback(const std::string & from, std::string & to,
                             boost::uint32_t codepage) {
	
	size_t skip = ((codepage == cp_utf16) ? 2u : 1u);
	
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

void to_utf8(const std::string & from, std::string & to, boost::uint32_t codepage) {
	
	if(codepage == cp_utf8) {
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
	
	size_t skip = ((codepage == cp_utf16) ? 2u : 1u);
	
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
				// invalid byte (sequence) - add a replacement char nd try the next byte
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
