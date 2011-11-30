
#ifndef INNOEXTRACT_SETUP_DATA_HPP
#define INNOEXTRACT_SETUP_DATA_HPP

#include <stddef.h>
#include <stdint.h>
#include <ctime>
#include <iosfwd>

#include "crypto/checksum.hpp"
#include "stream/chunk.hpp"
#include "stream/file.hpp"
#include "util/enum.hpp"
#include "util/flags.hpp"

namespace setup {

struct version;

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
		
		// obsolete:
		BZipped
	);
	
	stream::chunk chunk;
	
	stream::file file;
	
	timespec timestamp;
	
	uint32_t file_version_ms;
	uint32_t file_version_ls;
	
	flags options;
	
	void load(std::istream & is, const version & version);
	
};

} // namespace setup

NAMED_FLAGS(setup::data_entry::flags)

#endif // INNOEXTRACT_SETUP_DATA_HPP
