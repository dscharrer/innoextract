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

#ifndef INNOEXTRACT_LOADER_OFFSETS_HPP
#define INNOEXTRACT_LOADER_OFFSETS_HPP

#include <stdint.h>
#include <iosfwd>

#include "crypto/checksum.hpp"

namespace loader {

struct offsets {
	
	uint32_t exe_offset; //!< Offset of compressed setup.e32. 0 means there is no exe in this file.
	uint32_t exe_compressed_size; //!< Size of setup.e32 after compression (0 = unknown)
	uint32_t exe_uncompressed_size; //!< Size of setup.e32 before compression
	crypto::checksum exe_checksum; //!< Checksum of setup.e32 before compression
	
	uint32_t message_offset;
	
	/*!
		* Offset of embedded setup-0.bin data (the setup headers)
		* This points to a version string (see setup/Version.hpp) followed by a
		* compressed block of headers (see stream/BlockReader.hpp and setup/SetupHeader.hpp)
		*/
	uint32_t header_offset;
	
	/*!
		* Offset of embedded setup-1.bin data.
		* If this is zero, the setup data is stored in seprarate files.
		*/
	uint32_t data_offset;
	
	/*!
	* Try to find the setup loader offsets in the given file.
	*/
	void load(std::istream & is);
	
private:
	
	bool load_from_exe_file(std::istream & is);
	
	bool load_from_exe_resource(std::istream & is);
	
	bool load_offsets_at(std::istream & is, uint32_t pos);
	
};

} // namespace loader

#endif // INNOEXTRACT_LOADER_OFFSETS_HPP
