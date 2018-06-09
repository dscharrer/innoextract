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
 * Wrapper to read a single file from a chunk (\ref stream::chunk_reader).
 */
#ifndef INNOEXTRACT_STREAM_FILE_HPP
#define INNOEXTRACT_STREAM_FILE_HPP

#include <istream>

#include <boost/iostreams/chain.hpp>

#include "crypto/checksum.hpp"
#include "util/unique_ptr.hpp"

namespace stream {

enum compression_filter {
	NoFilter,
	InstructionFilter4108,
	InstructionFilter5200,
	InstructionFilter5309,
	ZlibFilter,
};

/*!
 * Information specifying a single file inside a compressed chunk.
 *
 * This data is stored in \ref setup::data_entry "data entries".
 *
 * Files specified by this struct can be read using \ref file_reader.
 */
struct file {
	
	boost::uint64_t    offset;   //!< Offset of this file within the decompressed chunk.
	boost::uint64_t    size;     //!< Pre-filter size of this file in the decompressed chunk.
	
	crypto::checksum   checksum; //!< Checksum for the file.
	
	compression_filter filter;   //!< Additional filter used before compression.
	
	bool operator<(const file & o) const;
	bool operator==(const file & o) const;
	
};

/*!
 * Wrapper to read a single file from a \ref chunk_reader.
 * Restrics the stream to the file size and applies the appropriate filters.
 */
class file_reader {
	
	typedef boost::iostreams::chain<boost::iostreams::input> base_type;
	
public:
	
	typedef std::istream                 type;
	typedef util::unique_ptr<type>::type pointer;
	typedef file                         file_t;
	
	/*!
	 * Wrap a \ref chunk_reader to read a single file.
	 *
	 * Only one wrapper can be used at the same time for each \c base.
	 *
	 * \param base     The chunk reader containing the file.
	 *                 It must already be positioned at the file's offset.
	 * \param file     Information specifying the file to read.
	 * \param checksum Optional pointer to a checksum that is updated as the file is read.
	 *                 The type of the checksum will be the same as that stored in the file
	 *                 struct.
	 *
	 * \return a pointer to a non-seekable input stream for the requested file.
	 */
	static pointer get(base_type & base, const file_t & file, crypto::checksum * checksum);
	
};

} // namespace stream

#endif // INNOEXTRACT_STREAM_FILE_HPP
