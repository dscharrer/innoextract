
#include <string>
#include <algorithm>
#include <iostream>

#include <boost/iostreams/concepts.hpp>
#include <boost/iostreams/read.hpp>

#include "Types.h"
#include "../shady/transform.hpp"

class inno_chunk_filter : public boost::iostreams::multichar_input_filter {
	
private:
	
	typedef boost::iostreams::multichar_input_filter base_type;
	
public:
	
	typedef base_type::char_type char_type;
	typedef base_type::category category;
	
	inno_chunk_filter() : pos(0), length(0) { }
	
	void checkCrc(u32 expected) const;
	
	template<typename Source>
	bool readChunk(Source & src) {
		
		std::cout << "[chunk] big read" << std::endl;
		
		u32 chunkCrc32 = *reinterpret_cast<u32 *>(buffer);
		std::streamsize nread = boost::iostreams::read(src, reinterpret_cast<char *>(&chunkCrc32), 4);
		if(nread == -1) {
			std::cout << "[chunk] end" << std::endl;
			return false;
		}	else if(nread != 4) {
			std::cout << "[chunk] unexpected block end" << std::endl;
			throw std::string("unexpected block end");
		}
		
		length = boost::iostreams::read(src, buffer, sizeof(buffer));
		if(length == (size_t)-1) {
			std::cout << "[chunk] unexpected chunk end" << std::endl;
			throw std::string("unexpected chunk end");
		}
		
		std::cout << "[chunk] -> length=" << length << std::endl;
		
		checkCrc(chunkCrc32);
		
		pos = 0;
		
		return true;
	}
	
	template<typename Source>
	std::streamsize read(Source & src, char * dest, std::streamsize n) {
		
		std::cout << "[chunk] small read " << n << std::endl;
		
		size_t read = 0;
		while(n) {
			
			if(pos == length && !readChunk(src)) {
				return read ? read : -1;
			}
			
			std::streamsize size = std::min(n, std::streamsize(length - pos));
			
			std::copy(buffer + pos, buffer + pos + size, dest);
			pos += size;
			dest += size, n -= size;
			read += size;
			
			std::cout << "[chunk] +" << size << "  remaining: " << (length - pos) << std::endl;
			
		}
		
		return read;
	}
	
private:
	
	size_t pos;
	size_t length;
	char buffer[4096];
	
};
