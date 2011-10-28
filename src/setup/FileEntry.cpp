
#include "setup/FileEntry.hpp"

#include "util/LoadingUtils.hpp"
#include "util/StoredEnum.hpp"

namespace {

enum FileCopyMode {
	cmNormal,
	cmIfDoesntExist,
	cmAlwaysOverwrite,
	cmAlwaysSkipIfSameOrOlder,
};

STORED_ENUM_MAP(StoredFileCopyMode, cmNormal,
	cmNormal,
	cmIfDoesntExist,
	cmAlwaysOverwrite,
	cmAlwaysSkipIfSameOrOlder,
);

STORED_ENUM_MAP(StoredFileType0, FileEntry::UserFile,
	FileEntry::UserFile,
	FileEntry::UninstExe,
);

// win32, before 5.0.0
STORED_ENUM_MAP(StoredFileType1, FileEntry::UserFile,
	FileEntry::UserFile,
	FileEntry::UninstExe,
	FileEntry::RegSvrExe,
);

} // anonymous namespace

NAMED_ENUM(FileCopyMode)

ENUM_NAMES(FileCopyMode, "File Copy Mode",
	"normal",
	"if doesn't exist",
	"always overwrite",
	"always skip if same or older",
)

void FileEntry::load(std::istream & is, const InnoVersion & version) {
	
	options = 0;
	
	if(version < INNO_VERSION(1, 3, 21)) {
		::load<uint32_t>(is); // uncompressed size of the file entry structure
	}
	
	is >> EncodedString(source, version.codepage());
	is >> EncodedString(destination, version.codepage());
	is >> EncodedString(installFontName, version.codepage());
	if(version >= INNO_VERSION(5, 2, 5)) {
		is >> EncodedString(strongAssemblyName, version.codepage());
	} else {
		strongAssemblyName.clear();
	}
	
	loadConditionData(is, version);
	
	loadVersionData(is, version);
	
	location = loadNumber<int32_t>(is, version.bits);
	attributes = loadNumber<uint32_t>(is, version.bits);
	externalSize = (version >= INNO_VERSION(4, 0, 0)) ? loadNumber<uint64_t>(is) : loadNumber<uint32_t>(is);
	
	if(version < INNO_VERSION(3, 0, 5)) {
		FileCopyMode copyMode = StoredEnum<StoredFileCopyMode>(is).get();
		switch(copyMode) {
			case cmNormal: options |= PromptIfOlder; break;
			case cmIfDoesntExist: options |= OnlyIfDoesntExist | PromptIfOlder; break;
			case cmAlwaysOverwrite: options |= IgnoreVersion | PromptIfOlder; break;
			case cmAlwaysSkipIfSameOrOlder: break;
		}
	}
	
	if(version >= INNO_VERSION(4, 1, 0)) {
		permission = loadNumber<int16_t>(is);
	} else {
		permission = -1;
	}
	
	StoredFlagReader<Options> flags(is);
	
	flags.add(ConfirmOverwrite);
	flags.add(NeverUninstall);
	flags.add(RestartReplace);
	flags.add(DeleteAfterInstall);
	if(version.bits != 16) {
		flags.add(RegisterServer);
		flags.add(RegisterTypeLib);
		flags.add(SharedFile);
	}
	if(version < INNO_VERSION(2, 0, 0)) {
		flags.add(IsReadmeFile);
	}
	flags.add(CompareTimeStamp);
	flags.add(FontIsNotTrueType);
	flags.add(SkipIfSourceDoesntExist);
	flags.add(OverwriteReadOnly);
	if(version >= INNO_VERSION(1, 3, 21)) {
		flags.add(OverwriteSameVersion);
		flags.add(CustomDestName);
	}
	if(version >= INNO_VERSION(1, 3, 25)) {
		flags.add(OnlyIfDestFileExists);
	}
	if(version >= INNO_VERSION(2, 0, 5)) {
		flags.add(NoRegError);
	}
	if(version >= INNO_VERSION(3, 0, 1)) {
		flags.add(UninsRestartDelete);
	}
	if(version >= INNO_VERSION(3, 0, 5)) {
		flags.add(OnlyIfDoesntExist);
		flags.add(IgnoreVersion);
		flags.add(PromptIfOlder);
	}
	if(version >= INNO_VERSION(3, 0, 8)) {
		flags.add(DontCopy);
	}
	if(version >= INNO_VERSION(4, 0, 5)) {
		flags.add(UninsRemoveReadOnly);
	}
	if(version >= INNO_VERSION(4, 1, 8)) {
		flags.add(RecurseSubDirsExternal);
	}
	if(version >= INNO_VERSION(4, 2, 1)) {
		flags.add(ReplaceSameVersionIfContentsDiffer);
	}
	if(version >= INNO_VERSION(4, 2, 5)) {
		flags.add(DontVerifyChecksum);
	}
	if(version >= INNO_VERSION(5, 0, 3)) {
		flags.add(UninsNoSharedFilePrompt);
	}
	if(version >= INNO_VERSION(5, 1, 0)) {
		flags.add(CreateAllSubDirs);
	}
	if(version >= INNO_VERSION(5, 1, 2)) {
		flags.add(Bits32);
		flags.add(Bits64);
	}
	if(version >= INNO_VERSION(5, 2, 0)) {
		flags.add(ExternalSizePreset);
		flags.add(SetNtfsCompression);
		flags.add(UnsetNtfsCompression);
	}
	if(version >= INNO_VERSION(5, 2, 5)) {
		flags.add(GacInstall);
	}
	
	options = flags.get();
	
	if(version >= INNO_VERSION(3, 0, 5) && version < INNO_VERSION(5, 0, 3)) {
		// TODO find out where this byte comes from
		int byte = is.get();
		std::cout << "read: " << PrintHex(byte) << std::endl;
		if(byte) {
			LogWarning << "unknown byte: " << byte;
		}
	}
	
	if(version.bits == 16 || version >= INNO_VERSION(5, 0, 0)) {
		type = StoredEnum<StoredFileType0>(is).get();
	} else {
		type = StoredEnum<StoredFileType1>(is).get();
	}
}

ENUM_NAMES(FileEntry::Options, "File Option",
	"confirm overwrite",
	"never uninstall",
	"restart replace",
	"delete after install",
	"register server",
	"register type lib",
	"shared file",
	"compare timestamp",
	"font isn't truetype",
	"skip if source doesn't exist",
	"overwrite readonly",
	"overwrite same version",
	"custom destination name",
	"only if destination exists",
	"no reg error",
	"uninstall restart delete",
	"only if doesn't exist",
	"ignore version",
	"prompt if older",
	"don't copy",
	"uninstall remove readonly",
	"recurse subdirectories external",
	"replace same version if contents differ",
	"don't verify checksum",
	"uninstall no shared file prompt",
	"create all sub dirs",
	"32 bit",
	"64 bit",
	"external size preset",
	"set ntfs compression",
	"unset ntfs compression",
	"gac install",
	"readme",
)

ENUM_NAMES(FileEntry::Type, "File Entry Type",
	"user file",
	"uninstaller exe",
	"reg server exe",
)
