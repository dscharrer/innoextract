
#ifndef INNOEXTRACT_BLOCKREADER_HPP
#define INNOEXTRACT_BLOCKREADER_HPP

#include <iostream>

#include "Version.hpp"

class BlockReader {
	
public:
	
	static std::istream * get(std::istream & base, const InnoVersion & version);
	
};

#endif // INNOEXTRACT_BLOCKREADER_HPP
