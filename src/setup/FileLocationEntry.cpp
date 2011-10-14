
#include "setup/FileLocationEntry.hpp"

#include "util/LoadingUtils.hpp"
#include "util/Output.hpp"
#include "util/StoredEnum.hpp"

void FileLocationEntry::load(std::istream & is, const InnoVersion & version) {
	
	firstSlice = loadNumber<u32>(is, version.bits);
	lastSlice = loadNumber<u32>(is, version.bits);
	
	chunkOffset = loadNumber<u32>(is);
	
	if(version >= INNO_VERSION(4, 0, 1)) {
		fileOffset = loadNumber<u64>(is);
	} else {
		fileOffset = 0;
	}
	
	if(version >= INNO_VERSION(4, 0, 0)) {
		fileSize = loadNumber<u64>(is);
		chunkSize = loadNumber<u64>(is);
	} else {
		fileSize = loadNumber<u32>(is);
		chunkSize = loadNumber<u32>(is);
	}
	
	if(version >= INNO_VERSION(5, 3, 9)) {
		is.read(checksum.sha1, sizeof(checksum.sha1)), checksum.type = Checksum::Sha1;
	} else if(version >= INNO_VERSION(4, 2, 0)) {
		is.read(checksum.md5, sizeof(checksum.md5)), checksum.type = Checksum::MD5;
	} else if(version >= INNO_VERSION(4, 0, 1)) {
		checksum.crc32 = loadNumber<u32>(is), checksum.type = Checksum::Crc32;
	} else {
		checksum.adler32 = loadNumber<u32>(is), checksum.type = Checksum::Adler32;
	}
	
	if(version.bits == 16) {
		
		u32 date = loadNumber<u32>(is); // milliseconds?
		
		// TODO this seems to be off by a few years:
		// expected ~ 2000-04-18, got 1991-07-28
		
		timestamp.tv_sec = date;
		timestamp.tv_nsec = 0;
		
	} else {
		
		s64 filetime = loadNumber<s64>(is);
		
		static const s64 FILETIME_OFFSET = 0x19DB1DED53E8000l;
		if(filetime < FILETIME_OFFSET) {
			warning << "unexpected filetime: " << filetime;
		}
		filetime -= FILETIME_OFFSET;
		
		timestamp.tv_sec = std::time_t(filetime / 10000000);
		timestamp.tv_nsec = long(filetime % 10000000) * 100;
	}
	
	fileVersionMS = loadNumber<u32>(is);
	fileVersionLS = loadNumber<u32>(is);
	
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
	}
	if(version >= INNO_VERSION(5, 1, 13)) {
		flags.add(SolidBreak);
	}
	
	options = flags.get();
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
