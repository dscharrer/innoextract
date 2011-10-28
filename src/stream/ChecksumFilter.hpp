
#ifndef INNOEXTRACT_CHECKSUMFILTER_HPP
#define INNOEXTRACT_CHECKSUMFILTER_HPP

#include <boost/iostreams/concepts.hpp>
#include <boost/iostreams/read.hpp>

#include "crypto/Hasher.hpp"

#include "util/Output.hpp"

class checksum_filter : public boost::iostreams::multichar_input_filter {
	
private:
	
	typedef boost::iostreams::multichar_input_filter base_type;
	
public:
	
	typedef base_type::char_type char_type;
	typedef base_type::category category;
	
	inline checksum_filter(Hasher * _hasher) : hasher(_hasher) { }
	inline checksum_filter(const checksum_filter & o) : hasher(o.hasher) { }
	
	template<typename Source>
	std::streamsize read(Source & src, char * dest, std::streamsize n) {
		
		std::streamsize nread = boost::iostreams::read(src, dest, n);
		
		if(nread != EOF) {
			hasher->update(dest, nread);
		}
		
		return nread;
	}
	
private:
	
	Hasher * hasher;
	
};

#endif // INNOEXTRACT_CHECKSUMFILTER_HPP
