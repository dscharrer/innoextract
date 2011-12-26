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

#ifndef INNOEXTRACT_STREAM_FILE_HPP
#define INNOEXTRACT_STREAM_FILE_HPP

#include <istream>

#include <boost/shared_ptr.hpp>
#include <boost/iostreams/chain.hpp>

#include "crypto/checksum.hpp"

namespace stream {

enum compression_filter {
	NoFilter,
	InstructionFilter4108,
	InstructionFilter5200,
	InstructionFilter5309,
};

struct file {
	
	uint64_t offset; //!< Offset of this file within the decompressed chunk.
	uint64_t size; //!< Decompressed size of this file.
	
	crypto::checksum checksum;
	
	compression_filter filter; //!< Additional filter used before compression.
	
	bool operator<(const file & o) const;
	bool operator==(const file & o) const;
	
};

class file_reader {
	
	typedef boost::iostreams::chain<boost::iostreams::input> base_type;
	
public:
	
	typedef std::istream type;
	typedef boost::shared_ptr<type> pointer;
	
	static pointer get(base_type & base, const file & file, crypto::checksum * checksum_output);
	
};

}

#endif // INNOEXTRACT_STREAM_FILE_HPP
