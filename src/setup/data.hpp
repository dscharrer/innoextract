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
 * Structures for file content entries stored in Inno Setup files.
 */
#ifndef INNOEXTRACT_SETUP_DATA_HPP
#define INNOEXTRACT_SETUP_DATA_HPP

#include <stddef.h>
#include <iosfwd>

#include <boost/cstdint.hpp>

#include "crypto/checksum.hpp"
#include "stream/chunk.hpp"
#include "stream/file.hpp"
#include "util/enum.hpp"
#include "util/flags.hpp"

namespace setup {

struct info;

struct data_entry {
	
	FLAGS(flags,
		
		VersionInfoValid,
		VersionInfoNotValid,
		TimeStampInUTC,
		IsUninstallerExe,
		CallInstructionOptimized,
		Touch,
		ChunkEncrypted,
		ChunkCompressed,
		SolidBreak,
		Sign,
		SignOnce,
		
		// obsolete:
		BZipped
	);
	
	stream::chunk chunk;
	
	stream::file file;
	
	boost::uint64_t uncompressed_size;
	
	boost::int64_t timestamp;
	boost::uint32_t timestamp_nsec;
	
	boost::uint64_t file_version;
	
	flags options;
	
	/*!
	 * Load one data entry.
	 *
	 * \note This function may not be thread-safe on all operating systems.
	 */
	void load(std::istream & is, const info & i);
	
};

} // namespace setup

NAMED_FLAGS(setup::data_entry::flags)

#endif // INNOEXTRACT_SETUP_DATA_HPP
