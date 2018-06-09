/*
 * Copyright (C) 2011-2018 Daniel Scharrer
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

#include "stream/file.hpp"

#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/filter/zlib.hpp>

#include "stream/checksum.hpp"
#include "stream/exefilter.hpp"
#include "stream/restrict.hpp"

namespace io = boost::iostreams;

namespace stream {

bool file::operator<(const stream::file & o) const {
	
	if(offset != o.offset) {
		return (offset < o.offset);
	} else if(size != o.size) {
		return (size < o.size);
	} else if(filter != o.filter) {
		return (filter < o.filter);
	}
	
	return false;
}

bool file::operator==(const file & o) const {
	return (offset == o.offset
	        && size == o.size
	        && filter == o.filter);
}


file_reader::pointer file_reader::get(base_type & base, const file & file,
                                      crypto::checksum * checksum) {
	
	util::unique_ptr<io::filtering_istream>::type result(new io::filtering_istream);
	
	if(file.filter == ZlibFilter) {
		result->push(io::zlib_decompressor(), 8192);
	}
	
	if(checksum) {
		result->push(stream::checksum_filter(checksum, file.checksum.type), 8192);
	}
	
	switch(file.filter) {
		case NoFilter: break;
		case InstructionFilter4108: result->push(stream::inno_exe_decoder_4108(), 8192); break;
		case InstructionFilter5200: result->push(stream::inno_exe_decoder_5200(false), 8192); break;
		case InstructionFilter5309: result->push(stream::inno_exe_decoder_5200(true), 8192); break;
		case ZlibFilter: /* applied *after* calculating the checksum */ break;
	}
	
	result->push(stream::restrict(base, file.size));
	
	result->exceptions(std::ios_base::badbit);
	
	return pointer(result.release());
}

} // namespace stream
