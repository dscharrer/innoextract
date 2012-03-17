/*
 * Copyright (C) 2011-2012 Daniel Scharrer
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

#include "setup/data.hpp"

#include "setup/version.hpp"
#include "util/load.hpp"
#include "util/log.hpp"
#include "util/storedenum.hpp"

namespace setup {

void data_entry::load(std::istream & is, const version & version) {
	
	chunk.first_slice = load_number<uint32_t>(is, version.bits);
	chunk.last_slice = load_number<uint32_t>(is, version.bits);
	if(version < INNO_VERSION(4, 0, 0)) {
		if(chunk.first_slice < 1 || chunk.last_slice < 1) {
			log_warning << "[file location] unexpected disk number: " << chunk.first_slice << " / "
			            << chunk.last_slice;
		} else {
			chunk.first_slice--, chunk.last_slice--;
		}
	}
	
	chunk.offset = load_number<uint32_t>(is);
	
	if(version >= INNO_VERSION(4, 0, 1)) {
		file.offset = load_number<uint64_t>(is);
	} else {
		file.offset = 0;
	}
	
	if(version >= INNO_VERSION(4, 0, 0)) {
		file.size = load_number<uint64_t>(is);
		chunk.size = load_number<uint64_t>(is);
	} else {
		file.size = load_number<uint32_t>(is);
		chunk.size = load_number<uint32_t>(is);
	}
	
	if(version >= INNO_VERSION(5, 3, 9)) {
		is.read(file.checksum.sha1, sizeof(file.checksum.sha1)), file.checksum.type = crypto::SHA1;
	} else if(version >= INNO_VERSION(4, 2, 0)) {
		is.read(file.checksum.md5, sizeof(file.checksum.md5)), file.checksum.type = crypto::MD5;
	} else if(version >= INNO_VERSION(4, 0, 1)) {
		file.checksum.crc32 = load_number<uint32_t>(is), file.checksum.type = crypto::CRC32;
	} else {
		file.checksum.adler32 = load_number<uint32_t>(is), file.checksum.type = crypto::Adler32;
	}
	
	if(version.bits == 16) {
		
		int32_t date = load_number<int32_t>(is); // milliseconds?
		
		// TODO this seems to be off by a few years:
		// expected ~ 2000-04-18, got 1991-07-28
		
		timestamp.tv_sec = date;
		timestamp.tv_nsec = 0;
		
	} else {
		
		int64_t filetime = load_number<int64_t>(is);
		
		static const int64_t FILETIME_OFFSET = 0x19DB1DED53E8000l;
		if(filetime < FILETIME_OFFSET) {
			log_warning << "[file location] unexpected filetime: " << filetime;
		}
		filetime -= FILETIME_OFFSET;
		
		timestamp.tv_sec = std::time_t(filetime / 10000000);
		timestamp.tv_nsec = int32_t(filetime % 10000000) * 100;
	}
	
	file_version_ms = load_number<uint32_t>(is);
	file_version_ls = load_number<uint32_t>(is);
	
	options = 0;
	
	stored_flag_reader<flags> flags(is, version.bits);
	
	flags.add(VersionInfoValid);
	flags.add(VersionInfoNotValid);
	if(version >= INNO_VERSION(2, 0, 17) && version < INNO_VERSION(4, 0, 1)) {
		flags.add(BZipped);
	}
	if(version >= INNO_VERSION(4, 0, 10)) {
		flags.add(TimeStampInUTC);
	}
	if(version >= INNO_VERSION(4, 1, 0)) {
		flags.add(IsUninstallerExe);
	}
	if(version >= INNO_VERSION(4, 1, 8)) {
		flags.add(CallInstructionOptimized);
	}
	if(version >= INNO_VERSION(4, 2, 0)) {
		flags.add(Touch);
	}
	if(version >= INNO_VERSION(4, 2, 2)) {
		flags.add(ChunkEncrypted);
	}
	if(version >= INNO_VERSION(4, 2, 5)) {
		flags.add(ChunkCompressed);
	} else {
		options |= ChunkCompressed;
	}
	if(version >= INNO_VERSION(5, 1, 13)) {
		flags.add(SolidBreak);
	}
	
	options |= flags;
	
	chunk.compression = (options & ChunkCompressed) ? stream::UnknownCompression : stream::Stored;
	if(options & BZipped) {
		options |= ChunkCompressed;
		chunk.compression = stream::BZip2;
	}
	
	chunk.encrypted = (options & ChunkEncrypted);
	
	if(options & CallInstructionOptimized) {
		if(version < INNO_VERSION(5, 2, 0)) {
			file.filter = stream::InstructionFilter4108;
		} else if(version < INNO_VERSION(5, 3, 9)) {
			file.filter = stream::InstructionFilter5200;
		} else {
			file.filter = stream::InstructionFilter5309;
		}
	} else {
		file.filter = stream::NoFilter;
	}
}

} // namespace setup;

NAMES(setup::data_entry::flags, "File Location Option",
	"version info valid",
	"version info not valid",
	"timestamp in UTC",
	"is uninstaller exe",
	"call instruction optimized",
	"touch",
	"chunk encrypted",
	"chunk compressed",
	"solid break",
	"bzipped",
)
