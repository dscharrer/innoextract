/*
 * Copyright (C) 2011 Daniel Scharrer
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

std::map<uint32_t, iconv_t> converters;

iconv_t get_converter(uint32_t codepage) {
	
	std::map<uint32_t, iconv_t>::iterator i = converters.find(codepage);
	
	if(i != converters.end()) {
		return i->second;
	}
	
	std::ostringstream oss;
	if(codepage == 1200) {
		// iconv's behavior for "UTF-16" is platform-dependant if there is no BOM.
		// There never is any BOM in Inno Setup files and it's always little-endian,
		// so we specify the exact encoding.
		oss << "UTF-16LE";
	} else {
		oss << "CP" << codepage;
	}
	
	return converters[codepage] = iconv_open("UTF-8", oss.str().c_str());
}

};

void binary_string::load(std::istream & is, std::string & target) {
	
	int32_t length = load_number<int32_t>(is);
	if(is.fail()) {
		return;
	}
	
	target.clear();
	
	while(length) {
		char buffer[10 * 1024];
		int32_t buf_size = std::min(length, int32_t(sizeof(buffer)));
		is.read(buffer, buf_size);
		target.append(buffer, size_t(buf_size));
		length -= buf_size;
	}
}

void encoded_string::load(std::istream & is, std::string & target, uint32_t codepage) {
	
	std::string temp;
	binary_string::load(is, temp);
	
	to_utf8(temp, target, codepage);
}

void to_utf8(const std::string & from, std::string & to, uint32_t codepage) {
	
	iconv_t converter = get_converter(codepage);
	
	/*
	 * Some iconv implementations declare the second parameter of iconv() as
	 * const char **, others as char **.
	 * Use this little hack to compile with both variants.
	 */
	struct inbuf_ {
		const char * buf;
		inbuf_(const char * data) : buf(data) { };
		operator const char **() { return &buf; };
		operator char **() { return const_cast<char **>(&buf); };
	} inbuf(from.data());
	
	size_t insize = from.size();
	
	size_t outbase = 0;
	
	if(!insize) {
		to.clear();
		return;
	}
	
	iconv(converter, NULL, NULL, NULL, NULL);
	
	while(insize) {
		
		to.resize(outbase + insize + 4);
		
		char * outbuf = &to[0] + outbase;
		size_t outsize = to.size() - outbase;
		
		size_t ret = iconv(converter, inbuf, &insize, &outbuf, &outsize);
		if(ret == size_t(-1) && errno != E2BIG) {
			log_error << "iconv error while converting from CP" << codepage << ": " << errno;
			to.clear();
			return;
		}
		
		outbase = to.size() - outsize;
	}
	
	to.resize(outbase);
}
