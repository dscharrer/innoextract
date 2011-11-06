
#include "setup/FileLocationEntry.hpp"

#include "util/LoadingUtils.hpp"
#include "util/Output.hpp"
#include "util/StoredEnum.hpp"

void FileLocationEntry::load(std::istream & is, const InnoVersion & version) {
	
	firstSlice = loadNumber<uint32_t>(is, version.bits);
	lastSlice = loadNumber<uint32_t>(is, version.bits);
	if(version < INNO_VERSION(4, 0, 0)) {
		if(firstSlice < 1 || lastSlice < 1) {
			LogWarning << "unexpected disk number: " << firstSlice << " / " << lastSlice;
		}
		firstSlice--, lastSlice--;
	}
	
	chunkOffset = loadNumber<uint32_t>(is);
	
	if(version >= INNO_VERSION(4, 0, 1)) {
		fileOffset = loadNumber<uint64_t>(is);
	} else {
		fileOffset = 0;
	}
	
	if(version >= INNO_VERSION(4, 0, 0)) {
		fileSize = loadNumber<uint64_t>(is);
		chunkSize = loadNumber<uint64_t>(is);
	} else {
		fileSize = loadNumber<uint32_t>(is);
		chunkSize = loadNumber<uint32_t>(is);
	}
	
	if(version >= INNO_VERSION(5, 3, 9)) {
		is.read(checksum.sha1, sizeof(checksum.sha1)), checksum.type = Checksum::Sha1;
	} else if(version >= INNO_VERSION(4, 2, 0)) {
		is.read(checksum.md5, sizeof(checksum.md5)), checksum.type = Checksum::MD5;
	} else if(version >= INNO_VERSION(4, 0, 1)) {
		checksum.crc32 = loadNumber<uint32_t>(is), checksum.type = Checksum::Crc32;
	} else {
		checksum.adler32 = loadNumber<uint32_t>(is), checksum.type = Checksum::Adler32;
	}
	
	if(version.bits == 16) {
		
		int32_t date = loadNumber<int32_t>(is); // milliseconds?
		
		// TODO this seems to be off by a few years:
		// expected ~ 2000-04-18, got 1991-07-28
		
		timestamp.tv_sec = date;
		timestamp.tv_nsec = 0;
		
	} else {
		
		int64_t filetime = loadNumber<int64_t>(is);
		
		static const int64_t FILETIME_OFFSET = 0x19DB1DED53E8000l;
		if(filetime < FILETIME_OFFSET) {
			LogWarning << "unexpected filetime: " << filetime;
		}
		filetime -= FILETIME_OFFSET;
		
		timestamp.tv_sec = std::time_t(filetime / 10000000);
		timestamp.tv_nsec = int32_t(filetime % 10000000) * 100;
	}
	
	fileVersionMS = loadNumber<uint32_t>(is);
	fileVersionLS = loadNumber<uint32_t>(is);
	
	options = 0;
	
	StoredFlagReader<Options> flags(is);
	
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
	
	options |= flags.get();
	
	if(options & BZipped) {
		options |= ChunkCompressed;
	}
}

ENUM_NAMES(FileLocationEntry::Options, "File Location Option",
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
