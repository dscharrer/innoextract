
#ifndef INNOEXTRACT_SETUP_FILELOCATIONENTRY_HPP
#define INNOEXTRACT_SETUP_FILELOCATIONENTRY_HPP

#include <stdint.h>
#include <ctime>
#include <iosfwd>

#include "crypto/Checksum.hpp"
#include "setup/SetupItem.hpp"
#include "setup/Version.hpp"
#include "util/Enum.hpp"
#include "util/Flags.hpp"

struct FileLocationEntry : public SetupItem {
	
	FLAGS(Options,
		
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
	
	size_t firstSlice;
	size_t lastSlice;
	
	uint32_t chunkOffset; //!< offset of the compressed chunk in firstSlice
	uint64_t chunkSize; //! total compressed size of the chunk
	
	uint64_t fileOffset; //!< offset of this file within the decompressed chunk
	uint64_t fileSize; //!< decompressed size of this file
	
	Checksum checksum;
	
	timespec timestamp;
	
	uint32_t fileVersionMS;
	uint32_t fileVersionLS;
	
	Options options;
	
	void load(std::istream & is, const InnoVersion & version);
	
};

FLAGS_OVERLOADS(FileLocationEntry::Options)
NAMED_ENUM(FileLocationEntry::Options)

#endif // INNOEXTRACT_SETUP_FILELOCATIONENTRY_HPP
