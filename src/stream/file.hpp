
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
