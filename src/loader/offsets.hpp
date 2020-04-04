/*
 * Copyright (C) 2011-2020 Daniel Scharrer
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
 * Functions to find Inno Setup data inside an executable.
 */
#ifndef INNOEXTRACT_LOADER_OFFSETS_HPP
#define INNOEXTRACT_LOADER_OFFSETS_HPP

#include <iosfwd>

#include <boost/cstdint.hpp>

#include "crypto/checksum.hpp"

namespace loader {

/*!
 * Bootstrap data for Inno Setup installers
 *
 * This struct contains information used by the Inno Setup loader to bootstrap the installer.
 * Some of these values are not available for all Inno Setup versions
 *
 * Inno Setup versions before \c 5.1.5 simply stored a magic number and offset to this bootstrap
 * data at a fixed position (\c 0x30) in the .exe file.
 *
 * Alternatively, there is no stored bootstrap information and data is stored in external files
 * while the main setup files contains only the version and headers (header_offset is \c 0).
 *
 * Newer versions use a PE/COFF resource entry to store this bootstrap information.
 */
struct offsets {
	
	/*!
	 * True if we have some indication that this is an Inno Setup file
	 */
	bool found_magic;
	
	/*!
	 * Offset of compressed `setup.e32` (the actual installer code)
	 *
	 * A value of \c 0 means there is no setup.e32 embedded in this file
	 */
	boost::uint32_t exe_offset;
	
	/*!
	 * Size of `setup.e32` after compression, in bytes
	 *
	 * A value of \c 0 means the executable size is not known
	 */
	boost::uint32_t exe_compressed_size;
	
	//! Size of `setup.e32` before compression, in bytes
	boost::uint32_t exe_uncompressed_size;
	
	/*!
	 * Checksum of `setup.e32` before compression
	 *
	 * Currently this is either a \ref crypto::CRC32 or \ref crypto::Adler32 checksum.
	 */
	crypto::checksum exe_checksum;
	
	//! Offset of embedded setup messages
	boost::uint32_t message_offset;
	
	/*!
	 * Offset of embedded `setup-0.bin` data (the setup headers)
	 *
	 * This points to a \ref setup::version followed by two compressed blocks of
	 * headers (see \ref stream::block_reader).
	 *
	 * The headers are described by various structs in the \ref setup namespace.
	 * The first header is always \ref setup::header.
	 *
	 * Loading the version and headers is done in \ref setup::info.
	 */
	boost::uint32_t header_offset;
	
	/*!
	 * Offset of embedded `setup-1.bin` data
	 *
	 * A value of \c 0 means that the setup data is stored in seprarate files.
	 *
	 * \ref stream::slice_reader provides a uniform interface to this data, no matter if it
	 * is embedded or split into multiple external files (called slices).
	 *
	 * The data is made up of one or more compressed or uncompressed chunks
	 * (read by \ref stream::chunk_reader) which in turn each contain the raw data for one or more file.
	 *
	 * The layout of the chunks and files is stored in the \ref setup::data_entry headers
	 * while the \ref setup::file_entry headers provide the filenames and meta information.
	 */
	boost::uint32_t data_offset;
	
	/*!
	 * \brief Find the setup loader offsets in a file
	 *
	 * Finding the headers always works - if there is no bootstrap information we assume that
	 * this is a file containing only the version and headers.
	 *
	 * \param is a seekable stream of the main installer file. This should be the file
	 *           containing the headers, which is almost always the .exe file.
	 */
	void load(std::istream & is);
	
private:
	
	bool load_from_exe_file(std::istream & is);
	
	bool load_from_exe_resource(std::istream & is);
	
	bool load_offsets_at(std::istream & is, boost::uint32_t pos);
	
};

} // namespace loader

#endif // INNOEXTRACT_LOADER_OFFSETS_HPP
