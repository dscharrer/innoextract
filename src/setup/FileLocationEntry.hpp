
#ifndef INNOEXTRACT_SETUP_FILELOCATIONENTRY_HPP
#define INNOEXTRACT_SETUP_FILELOCATIONENTRY_HPP

#include <ctime>
#include <iostream>

#include "setup/SetupItem.hpp"
#include "setup/Version.hpp"
#include "util/Checksum.hpp"
#include "util/Enum.hpp"
#include "util/Flags.hpp"
#include "util/Types.hpp"

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
	
	size_t startOffset;
	
	u64 chunkSubOffset;
	
	u64 originalSize;
	
	u64 chunkCompressedSize;
	
	Checksum checksum;
	
	timespec timestamp;
	
	u32 fileVersionMS;
	u32 fileVersionLS;
	
	Options options;
	
	void load(std::istream & is, const InnoVersion & version);
	
};

FLAGS_OVERLOADS(FileLocationEntry::Options)
NAMED_ENUM(FileLocationEntry::Options)

#endif // INNOEXTRACT_SETUP_FILELOCATIONENTRY_HPP
