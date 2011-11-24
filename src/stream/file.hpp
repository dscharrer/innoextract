
#ifndef INNOEXTRACT_STREAM_FILE_HPP
#define INNOEXTRACT_STREAM_FILE_HPP

#include <istream>

#include <boost/shared_ptr.hpp>
#include <boost/iostreams/chain.hpp>

class Hasher;
struct FileLocationEntry;
struct InnoVersion;

namespace stream {

class file_reader {
	
	typedef boost::iostreams::chain<boost::iostreams::input> base_type;
	
public:
	
	typedef std::istream type;
	typedef boost::shared_ptr<type> pointer;
	
	static pointer get(base_type & base, const FileLocationEntry & location,
	                   const InnoVersion & version, Hasher & hasher);
	
};

}

#endif // INNOEXTRACT_STREAM_FILE_HPP
