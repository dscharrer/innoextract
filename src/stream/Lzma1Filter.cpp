
#include "stream/Lzma1Filter.hpp"

#include <iostream>

#include <lzma.h>

#include "util/Types.hpp"

using std::cout;
using std::endl;

inno_lzma1_decompressor_impl::inno_lzma1_decompressor_impl() : nread(0), stream(NULL), eof(false) { }

inno_lzma1_decompressor_impl::~inno_lzma1_decompressor_impl() { close(); }

bool inno_lzma1_decompressor_impl::filter(const char * & begin_in, const char * end_in, char * & begin_out, char * end_out, bool flush) {
	(void)flush;
	
	size_t bufsize_in = (end_in - begin_in), bufsize_out = (end_out - begin_out);
	
	// Read enough bytes to decode the header.
	while(nread != 5) {
		if(begin_in == end_in) {
			return true;
		}
		buf[nread++] = *begin_in++;
	}
	
	lzma_stream * strm = reinterpret_cast<lzma_stream *>(stream);
	
	// Decode the header.
	if(!strm) {
		
		stream = strm = new lzma_stream;
		
		// Initialize the stream;
		lzma_stream tmp = LZMA_STREAM_INIT;
		*strm = tmp;
		
		strm->allocator = NULL;
		
		lzma_options_lzma options;
		options.preset_dict = NULL;
		
		u8 properties = buf[0];
		if(properties > (9 * 5 * 5)) {
			delete strm, stream = NULL;
			throw lzma_error(LZMA_FORMAT_ERROR);
		}
		options.pb = properties / (9 * 5);
		options.lp = (properties % (9 * 5)) / 9;
		options.lc = properties % 9;
		
		options.dict_size = 0;
		for(size_t i = 0; i < 4; i++) {
			options.dict_size += u32(buf[i + 1]) << (i * 8);
		}
		if(options.dict_size > (1 << 28)) {
			delete strm, stream = NULL;
			throw lzma_error(LZMA_FORMAT_ERROR);
		}
		
		cout << "[lzma] lc=" << options.lc << "  lp=" << options.lp << "  pb=" << options.pb << "  dict size: " << options.dict_size << endl;
		
		lzma_filter filters[2] = { { LZMA_FILTER_LZMA1,  &options }, { LZMA_VLI_UNKNOWN } };
		
		lzma_ret ret = lzma_raw_decoder(strm, filters);
		if(ret != LZMA_OK) {
			delete strm, stream = NULL;
			throw lzma_error(ret);
		}
		
	}
	
	strm->next_in = reinterpret_cast<const uint8_t *>(begin_in);
	strm->avail_in = end_in - begin_in;
	
	strm->next_out = reinterpret_cast<uint8_t *>(begin_out);
	strm->avail_out = end_out - begin_out;
	
	lzma_ret ret = lzma_code(strm, LZMA_RUN);
	
	cout << "[lzma] decompressed " << (reinterpret_cast<const char *>(strm->next_in) - begin_in) << " / " << bufsize_in << " -> " << (reinterpret_cast<char *>(strm->next_out) - begin_out) << " / " << bufsize_out << "  ret=" << ret << endl;
	
	begin_in = reinterpret_cast<const char *>(strm->next_in);
	
	begin_out = reinterpret_cast<char *>(strm->next_out);
	
	if(ret != LZMA_OK && ret != LZMA_STREAM_END && ret != LZMA_BUF_ERROR) {
		throw lzma_error(ret);
	}
	
	if(ret == LZMA_STREAM_END) {
		cout << "[lzma] end" << endl;
		return false;
	}
	
	return true;
}

void inno_lzma1_decompressor_impl::close() {
	
	if(stream) {
		
		cout << "[lzma] close" << endl;
		
		lzma_stream * strm = reinterpret_cast<lzma_stream *>(stream);
		
		lzma_end(strm);
		
		delete strm, stream = NULL;
	}
	
	nread = 0;
	eof = false;
}
