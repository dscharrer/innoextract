
#ifndef INNOEXTRACT_STREAM_BLOCK_HPP
#define INNOEXTRACT_STREAM_BLOCK_HPP

#include <iosfwd>

#include <boost/shared_ptr.hpp>

struct inno_version;

namespace stream {

struct block_error : public std::ios_base::failure {
	
	inline block_error(std::string msg) : failure(msg) { }
	
};

/*!
 * Reads a compressed and checksumed block of data used to store the setup headers.
 */
class block_reader {
	
public:
	
	typedef std::istream type;
	typedef boost::shared_ptr<type> pointer;
	
	static pointer get(std::istream & base, const inno_version & version);
	
};

} // namespace stream

#endif // INNOEXTRACT_STREAM_BLOCK_HPP
