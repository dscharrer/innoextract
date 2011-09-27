
#include "LoadingUtils.hpp"

#include <iterator>

#include <iconv.h>
#include <errno.h>

#include "Output.hpp"
#include "Utils.hpp"

void BinaryString::loadInto(std::istream & is, std::string & target) {
	
	size_t length = loadNumber<u32>(is);
	if(is.fail()) {
		return;
	}
	
	target.resize(length);
	is.read(&target[0], length);
}

void convert(iconv_t converter, const std::string & from, std::string & to) {
	
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
			error << "iconv error";
			to.clear();
			return;
		}
		
		outbase = to.size() - outsize;
	}
	
	to.resize(outbase);
	
}

void AnsiString::loadInto(std::istream & is, std::string & target) {
	
	std::string temp;
	BinaryString::loadInto(is, temp);
	
	static iconv_t converter = NULL;
	if(!converter) {
		converter = iconv_open("UTF-8", "CP1252");
		if(!converter) {
			error << "missing CP1252 -> UTF-8 converter";
		}
	}
	
	convert(converter, temp, target);
}

void WideString::loadInto(std::istream & is, std::string & target) {
	
	std::string temp;
	BinaryString::loadInto(is, temp);
	
	static iconv_t converter = NULL;
	if(!converter) {
		converter = iconv_open("UTF-8", "UTF-16");
		if(!converter) {
			error << "missing UTF-16 -> UTF-8 converter";
		}
	}
	
	convert(converter, temp, target);
	
}
