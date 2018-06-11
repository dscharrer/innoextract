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

/*!
 * \file
 *
 * LZMA 1 and 2 (aka xz) descompression filters to be used with boost::iostreams.
 */
#ifndef INNOEXTRACT_STREAM_LZMA_HPP
#define INNOEXTRACT_STREAM_LZMA_HPP

#include "configure.hpp"

#if INNOEXTRACT_HAVE_LZMA

#include <stddef.h>
#include <iosfwd>

#include <boost/iostreams/filter/symmetric.hpp>
#include <boost/noncopyable.hpp>

namespace stream {

//! Error thrown if there was en error in an LZMA stream
struct lzma_error : public std::ios_base::failure {
	
	lzma_error(const std::string & msg, int code)
		: std::ios_base::failure(msg), error_code(code) { }
	
	//! \return the liblzma code for the error.
	int error() const { return error_code; }
	
private:
	
	int error_code;
};

class lzma_decompressor_impl_base : private boost::noncopyable {
	
public:
	
	typedef char char_type;
	
	~lzma_decompressor_impl_base() { close(); }
	
	bool filter(const char * & begin_in, const char * end_in,
	            char * & begin_out, char * end_out, bool flush);
	
	void close();
	
protected:
	
	//! Abstract base class, subclasses need to intialize stream.
	lzma_decompressor_impl_base() : stream(NULL) { }
	
	void * stream;
	
};

class inno_lzma1_decompressor_impl : public lzma_decompressor_impl_base {
	
public:
	
	inno_lzma1_decompressor_impl() : nread(0) { }
	
	bool filter(const char * & begin_in, const char * end_in,
	            char * & begin_out, char * end_out, bool flush);
	
	void close() { lzma_decompressor_impl_base::close(), nread = 0; }
	
private:
	
	size_t nread; //! Number of bytes read into header.
	char header[5];
	
};

class inno_lzma2_decompressor_impl : public lzma_decompressor_impl_base {
	
public:
	
	bool filter(const char * & begin_in, const char * end_in,
	            char * & begin_out, char * end_out, bool flush);
	
};

template <class Impl, class Allocator = std::allocator<typename Impl::char_type> >
class lzma_decompressor : public boost::iostreams::symmetric_filter<Impl, Allocator> {
	
public:
	
	explicit lzma_decompressor(int buffer_size = boost::iostreams::default_device_buffer_size)
		: boost::iostreams::symmetric_filter<Impl, Allocator>(buffer_size) { }
	
};

/*!
 * A filter that decompressess LZMA1 streams found in Inno Setup installers,
 * to be used with boost::iostreams.
 *
 * The LZMA1 streams used by Inno Setup differ slightly from the LZMA Alone file format:
 * The stream header only stores the properties (lc, lp, pb) and the dictionary size and
 * is missing the uncompressed size field. The fiels that are present are encoded
 * identically.
 */
typedef lzma_decompressor<inno_lzma1_decompressor_impl> inno_lzma1_decompressor;

/*!
 * A filter that decompressess LZMA2 streams found in Inno Setup installers,
 * to be used with boost::iostreams.
 *
 * Inno Setup uses raw LZMA2 streams.
 * (preceded only by the dictionary size encoded as one byte)
 */
typedef lzma_decompressor<inno_lzma2_decompressor_impl> inno_lzma2_decompressor;

} // namespace stream

#endif // INNOEXTRACT_HAVE_LZMA

#endif // INNOEXTRACT_STREAM_LZMA_HPP
