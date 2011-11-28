
#ifndef INNOEXTRACT_SETUP_DATA_HPP
#define INNOEXTRACT_SETUP_DATA_HPP

#include <stddef.h>
#include <stdint.h>
#include <ctime>
#include <iosfwd>

#include "crypto/checksum.hpp"
#include "setup/SetupItem.hpp"
#include "setup/version.hpp"
#include "util/enum.hpp"
#include "util/flags.hpp"

namespace setup {

struct data_entry : public SetupItem {
	
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
	
	size_t first_slice;
	size_t last_slice;
	
	uint32_t chunk_offset; //!< offset of the compressed chunk in firstSlice
	uint64_t chunk_size; //! total compressed size of the chunk
	
	uint64_t file_offset; //!< offset of this file within the decompressed chunk
	uint64_t file_size; //!< decompressed size of this file
	
	crypto::checksum checksum;
	
	timespec timestamp;
	
	uint32_t file_version_ms;
	uint32_t file_version_ls;
	
	flags options;
	
	void load(std::istream & is, const inno_version & version);
	
};

} // namespace setup

FLAGS_OVERLOADS(setup::data_entry::flags)
NAMED_ENUM(setup::data_entry::flags)

#endif // INNOEXTRACT_SETUP_DATA_HPP
