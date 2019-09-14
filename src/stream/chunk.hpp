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

/*!
 * \file
 *
 * Wrapper to read and decompress a chunk from a \ref stream::slice_reader.
 *
 * A chunk consists of one compression stream (one of \ref stream::compression_method) and
 * contains one or more \ref stream::file "files".
 * Files may also have additional filters managed by \ref stream::file_reader.
 */
#ifndef INNOEXTRACT_STREAM_CHUNK_HPP
#define INNOEXTRACT_STREAM_CHUNK_HPP

#include <stddef.h>
#include <ios>
#include <string>

#include <boost/cstdint.hpp>
#include <boost/iostreams/chain.hpp>

#include "util/enum.hpp"
#include "util/unique_ptr.hpp"

namespace stream {

class slice_reader;

//! Error thrown by \ref chunk_reader::get if there was a problem.
struct chunk_error : public std::ios_base::failure {
	
	explicit chunk_error(const std::string & msg) : std::ios_base::failure(msg) { }
	
};

//! Compression methods supported by chunks.
enum compression_method {
	Stored,
	Zlib,
	BZip2,
	LZMA1,
	LZMA2,
	UnknownCompression
};

//! Encryption methods supported by chunks.
enum encryption_method {
	Plaintext,
	ARC4_MD5,
	ARC4_SHA1,
};

/*!
 * Information specifying a compressed chunk.
 *
 * This data is stored in \ref setup::data_entry "data entries".
 *
 * Chunks specified by this struct can be read using \ref chunk_reader.
 */
struct chunk {
	
	boost::uint32_t first_slice;    //!< Slice where the chunk starts.
	boost::uint32_t last_slice;     //!< Slice where the chunk ends.
	
	boost::uint32_t sort_offset;
	
	boost::uint32_t offset;         //!< Offset of the compressed chunk in firstSlice.
	boost::uint64_t size;           //! Total compressed size of the chunk.
	
	compression_method compression; //!< Compression method used by the chunk.
	encryption_method encryption;   //!< Encryption method used by the chunk.
	
	bool operator<(const chunk & o) const;
	bool operator==(const chunk & o) const;
	
};

class silce_source;

/*!
 * Wrapper to read and decompress a chunk from a \ref slice_reader.
 * Restrics the stream to the chunk size and applies the appropriate decompression.
 */
class chunk_reader {
	
public:
	
	typedef boost::iostreams::chain<boost::iostreams::input> type;
	typedef util::unique_ptr<type>::type                     pointer;
	
	/*!
	 * Wrap a \ref slice_reader to read and decompress a single chunk.
	 *
	 * Only one wrapper can be used at the same time for each \c base.
	 *
	 * \param base  The slice reader for the setup file(s).
	 * \param chunk Information specifying the chunk to read.
	 * \param key   Key used for encrypted chunks.
	 *
	 * \throws chunk_error if the chunk header could not be read or was invalid,
	 *                     or if the chunk compression is not supported by this build.
	 *
	 * \return a pointer to a non-seekable input filter chain for the requested file.
	 */
	static pointer get(slice_reader & base, const ::stream::chunk & chunk, const std::string & key);
	
};

} // namespace stream

NAMED_ENUM(stream::compression_method)

NAMED_ENUM(stream::encryption_method)

#endif // INNOEXTRACT_STREAM_CHUNK_HPP
