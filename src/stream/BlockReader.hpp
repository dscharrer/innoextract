
#ifndef INNOEXTRACT_STREAM_BLOCKREADER_HPP
#define INNOEXTRACT_STREAM_BLOCKREADER_HPP

#include <iostream>

#include "setup/Version.hpp"

class BlockReader {
	
public:
	
	static std::istream * get(std::istream & base, const InnoVersion & version);
	
};

#endif // INNOEXTRACT_STREAM_BLOCKREADER_HPP
