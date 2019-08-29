/*
 * Copyright (C) 2011-2019 Daniel Scharrer
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

#include "stream/lzma.hpp"

#include <boost/cstdint.hpp>

#include <lzma.h>

#include "util/endian.hpp"
#include "util/load.hpp"

namespace stream {

static lzma_stream * init_raw_lzma_stream(lzma_vli filter, lzma_options_lzma & options) {
	
	options.preset_dict = NULL;
	
	lzma_stream * strm = new lzma_stream;
	lzma_stream tmp = LZMA_STREAM_INIT;
	*strm = tmp;
	strm->allocator = NULL;
	
	const lzma_filter filters[2] = { { filter,  &options }, { LZMA_VLI_UNKNOWN, NULL } };
	lzma_ret ret = lzma_raw_decoder(strm, filters);
	if(ret != LZMA_OK) {
		delete strm;
		throw lzma_error("inno lzma init error", ret);
	}
	
	return strm;
}

bool lzma_decompressor_impl_base::filter(const char * & begin_in, const char * end_in,
                                         char * & begin_out, char * end_out, bool flush) {
	
	lzma_stream * strm = static_cast<lzma_stream *>(stream);
	
	strm->next_in = reinterpret_cast<const boost::uint8_t *>(begin_in);
	strm->avail_in = size_t(end_in - begin_in);
	
	strm->next_out = reinterpret_cast<boost::uint8_t *>(begin_out);
	strm->avail_out = size_t(end_out - begin_out);
	
	lzma_ret ret = lzma_code(strm, LZMA_RUN);
	
	if(flush && ret == LZMA_BUF_ERROR && strm->avail_out > 0) {
		throw lzma_error("truncated lzma stream", ret);
	}
	
	begin_in = reinterpret_cast<const char *>(strm->next_in);
	begin_out = reinterpret_cast<char *>(strm->next_out);
	
	if(ret != LZMA_OK && ret != LZMA_STREAM_END && ret != LZMA_BUF_ERROR) {
		throw lzma_error("lzma decrompression error", ret);
	}
	
	return (ret != LZMA_STREAM_END);
}

void lzma_decompressor_impl_base::close() {
	
	if(stream) {
		lzma_stream * strm = static_cast<lzma_stream *>(stream);
		lzma_end(strm);
		delete strm, stream = NULL;
	}
}

bool inno_lzma1_decompressor_impl::filter(const char * & begin_in, const char * end_in,
                                          char * & begin_out, char * end_out, bool flush) {
	
	// Decode the header.
	if(!stream) {
		
		// Read enough bytes to decode the header.
		while(nread != 5) {
			if(begin_in == end_in) {
				return true;
			}
			header[nread++] = *begin_in++;
		}
		
		lzma_options_lzma options;
		
		boost::uint8_t properties = boost::uint8_t(header[0]);
		if(properties > (9 * 5 * 5)) {
			throw lzma_error("inno lzma1 property error", LZMA_FORMAT_ERROR);
		}
		options.pb = boost::uint32_t(properties / (9 * 5));
		options.lp = boost::uint32_t((properties % (9 * 5)) / 9);
		options.lc = boost::uint32_t(properties % 9);
		
		options.dict_size = util::little_endian::load<boost::uint32_t>(header + 1);
		
		stream = init_raw_lzma_stream(LZMA_FILTER_LZMA1, options);
	}
	
	return lzma_decompressor_impl_base::filter(begin_in, end_in, begin_out, end_out, flush);
}

bool inno_lzma2_decompressor_impl::filter(const char * & begin_in, const char * end_in,
                                          char * & begin_out, char * end_out, bool flush) {
	
	// Decode the header.
	if(!stream) {
		
		if(begin_in == end_in) {
			return true;
		}
		
		lzma_options_lzma options;
		
		boost::uint8_t prop = boost::uint8_t(*begin_in++);
		if(prop > 40) {
			throw lzma_error("inno lzma2 property error", LZMA_FORMAT_ERROR);
		}
		
		if(prop == 40) {
			options.dict_size = 0xffffffff;
		} else {
			options.dict_size = ((boost::uint32_t(2) | boost::uint32_t((prop) & 1)) << ((prop) / 2 + 11));
		}
		
		stream = init_raw_lzma_stream(LZMA_FILTER_LZMA2, options);
	}
	
	return lzma_decompressor_impl_base::filter(begin_in, end_in, begin_out, end_out, flush);
}

} // namespace stream
