
#ifndef INNOEXTRACT_STREAM_FILE_HPP
#define INNOEXTRACT_STREAM_FILE_HPP

#include <istream>

#include <boost/shared_ptr.hpp>
#include <boost/iostreams/chain.hpp>

namespace crypto { struct checksum; }
namespace setup { struct data_entry; }
struct inno_version;

namespace stream {

class file_reader {
	
	typedef boost::iostreams::chain<boost::iostreams::input> base_type;
	
public:
	
	typedef std::istream type;
	typedef boost::shared_ptr<type> pointer;
	
	static pointer get(base_type & base, const setup::data_entry & location,
	                   const inno_version & version, crypto::checksum * checksum_output);
	
};

}

#endif // INNOEXTRACT_STREAM_FILE_HPP
