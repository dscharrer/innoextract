
#ifndef INNOEXTRACT_STREAM_BLOCKREADER_HPP
#define INNOEXTRACT_STREAM_BLOCKREADER_HPP

#include <iostream>

#include "setup/Version.hpp"

/*!
 * Reads a compressed and checksumed block of data used to store the setup headers.
 */
class BlockReader {
	
public:
	
	static std::istream * get(std::istream & base, const InnoVersion & version);
	
};

#endif // INNOEXTRACT_STREAM_BLOCKREADER_HPP
