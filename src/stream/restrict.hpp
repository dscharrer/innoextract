/*
 * Copyright (C) 2013-2019 Daniel Scharrer
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

/*!
 * \file
 *
 * Wrapper class for a boost::iostreams-compatible source that can be used to restrict
 * sources to appear smaller than they really are.
 */
#ifndef INNOEXTRACT_STREAM_RESTRICT_HPP
#define INNOEXTRACT_STREAM_RESTRICT_HPP

#include <boost/cstdint.hpp>
#include <boost/iostreams/concepts.hpp>
#include <boost/iostreams/read.hpp>

namespace stream {

//! Like boost::iostreams::restriction, but always has a 64-bit counter.
template <typename BaseSource>
class restricted_source : public boost::iostreams::source {
	
	BaseSource &    base;      //!< The base source to read from.
	boost::uint64_t remaining; //!< Number of bytes remaining in the restricted source.
	
public:
	
	restricted_source(const restricted_source & o)
		: base(o.base), remaining(o.remaining) { }
	
	restricted_source(BaseSource & source, boost::uint64_t size)
		: base(source), remaining(size) { }
	
	std::streamsize read(char * buffer, std::streamsize bytes) {
		
		if(bytes <= 0) {
			return 0;
		}
		
		// Restrict the number of bytes to read
		bytes = std::streamsize(std::min(boost::uint64_t(bytes), remaining));
		if(bytes == 0) {
			return -1; // End of the restricted source reached
		}
		
		std::streamsize nread = boost::iostreams::read(base, buffer, bytes);
		
		// Remember how many bytes were read so far
		if(nread > 0) {
			remaining -= std::min(boost::uint64_t(nread), remaining);
		}
		
		return nread;
	}
	
};

/*!
 * Restricts a source to a specific size from the current position and makes
 * it non-seekable.
 *
 * Like boost::iostreams::restrict, but always has a 64-bit counter.
 */
template <typename BaseSource>
restricted_source<BaseSource> restrict(BaseSource & source, boost::uint64_t size) {
	return restricted_source<BaseSource>(source, size);
}

} // namespace stream

#endif // INNOEXTRACT_STREAM_RESTRICT_HPP
