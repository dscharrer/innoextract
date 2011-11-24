
#include "util/load.hpp"

#include <iterator>
#include <map>
#include <sstream>

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
		oss << "UTF-16";
	} else {
		oss << "CP" << codepage;
	}
	
	return converters[codepage] = iconv_open("UTF-8", oss.str().c_str());
}

};

void binary_string::load(std::istream & is, std::string & target) {
	
	int32_t length = load_number<int32_t>(is);
	if(is.fail() || length < 0) {
		return;
	}
	
	// TODO read / allocate huge strings in chunks
	target.resize(size_t(length));
	is.read(&target[0], length);
}

void encoded_string::load(std::istream & is, std::string & target, uint32_t codepage) {
	
	std::string temp;
	binary_string::load(is, temp);
	
	to_utf8(temp, target, codepage);
}

void to_utf8(const std::string & from, std::string & to, uint32_t codepage) {
	
	iconv_t converter = get_converter(codepage);
	
	const char * inbuf = from.data();
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
		
		size_t ret = iconv(converter, const_cast<char**>(&inbuf), &insize, &outbuf, &outsize);
		if(ret == size_t(-1) && errno != E2BIG) {
			log_error << "iconv error while converting from CP" << codepage << ": " << errno;
			to.clear();
			return;
		}
		
		outbase = to.size() - outsize;
	}
	
	to.resize(outbase);
}
