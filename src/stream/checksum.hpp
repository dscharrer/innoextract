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

#ifndef INNOEXTRACT_STREAM_CHECKSUM_HPP
#define INNOEXTRACT_STREAM_CHECKSUM_HPP

#include <boost/iostreams/concepts.hpp>
#include <boost/iostreams/read.hpp>

#include "crypto/checksum.hpp"
#include "crypto/hasher.hpp"

namespace stream {

class checksum_filter : public boost::iostreams::multichar_input_filter {
	
private:
	
	typedef boost::iostreams::multichar_input_filter base_type;
	
public:
	
	typedef base_type::char_type char_type;
	typedef base_type::category category;
	
	inline checksum_filter(crypto::checksum * output, crypto::checksum_type type) : output(output) {
		hasher.init(type);
	}
	inline checksum_filter(const checksum_filter & o) : hasher(o.hasher), output(o.output) { }
	
	template<typename Source>
	std::streamsize read(Source & src, char * dest, std::streamsize n) {
		
		std::streamsize nread = boost::iostreams::read(src, dest, n);
		
		if(nread > 0) {
			hasher.update(dest, size_t(nread));
		} else if(output) {
			*output = hasher.finalize();
			output = NULL;
		}
		
		return nread;
	}
	
private:
	
	crypto::hasher hasher;
	
	crypto::checksum * output;
	
};

} // namespace stream

#endif // INNOEXTRACT_STREAM_CHECKSUM_HPP
