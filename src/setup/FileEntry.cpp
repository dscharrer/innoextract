
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
	
	if(version <= INNO_VERSION(1, 2, 16)) {
		::load<u32>(is); // uncompressed size of the file entry structure
	}
	
	is >> EncodedString(source, version.codepage());
	is >> EncodedString(destination, version.codepage());
	is >> EncodedString(installFontName, version.codepage());
	if(version >= INNO_VERSION(5, 2, 5)) {
		is >> EncodedString(strongAssemblyName, version.codepage());
	} else {
		strongAssemblyName.clear();
	}
	condition.load(is, version);
	tasks.load(is, version);
	
	minVersion.load(is, version);
	onlyBelowVersion.load(is, version);
	
	location = loadNumber<s32>(is, version.bits);
	attributes = loadNumber<u32>(is, version.bits);
	externalSize = (version >= INNO_VERSION(4, 0, 0)) ? loadNumber<u64>(is) : loadNumber<u32>(is);
	
	if(version < INNO_VERSION(3, 0, 5)) {
		FileCopyMode copyMode = StoredEnum<StoredFileCopyMode>(is).get();
		// TODO this might be wrong
		switch(copyMode) {
			case cmNormal: options |= foPromptIfOlder; break;
			case cmIfDoesntExist: options |= foOnlyIfDoesntExist | foPromptIfOlder; break;
			case cmAlwaysOverwrite: options |= foIgnoreVersion | foPromptIfOlder; break;
			case cmAlwaysSkipIfSameOrOlder: break;
		}
	}
	
	if(version >= INNO_VERSION(4, 1, 0)) {
		permission = loadNumber<s16>(is);
	} else {
		permission = -1;
	}
	
	StoredFlagReader<FileOptions> flags(is);
	
	flags.add(foConfirmOverwrite);
	flags.add(foUninsNeverUninstall);
	flags.add(foRestartReplace);
	flags.add(foDeleteAfterInstall);
	if(version.bits != 16) {
		flags.add(foRegisterServer);
		flags.add(foRegisterTypeLib);
		flags.add(foSharedFile);
	}
	if(version < INNO_VERSION(1, 3, 26)) {
		flags.add(foIsReadmeFile);
	}
	flags.add(foCompareTimeStamp);
	flags.add(foFontIsntTrueType);
	flags.add(foSkipIfSourceDoesntExist);
	flags.add(foOverwriteReadOnly);
	if(version > INNO_VERSION(1, 2, 16)) {
		flags.add(foOverwriteSameVersion);
		flags.add(foCustomDestName);
		flags.add(foOnlyIfDestFileExists);
	}
	if(version > INNO_VERSION(1, 3, 26)) {
		flags.add(foNoRegError);
	}
	if(version >= INNO_VERSION(3, 0, 1)) {
		flags.add(foUninsRestartDelete);
	}
	if(version >= INNO_VERSION(3, 0, 5)) {
		flags.add(foOnlyIfDoesntExist);
		flags.add(foIgnoreVersion);
		flags.add(foPromptIfOlder);
	}
	if(version >= INNO_VERSION(3, 0, 8)) {
		flags.add(foDontCopy);
	}
	if(version >= INNO_VERSION(4, 0, 5)) {
		flags.add(foUninsRemoveReadOnly);
	}
	if(version >= INNO_VERSION(4, 1, 8)) {
		flags.add(foRecurseSubDirsExternal);
	}
	if(version >= INNO_VERSION(4, 2, 1)) {
		flags.add(foReplaceSameVersionIfContentsDiffer);
	}
	if(version >= INNO_VERSION(4, 2, 5)) {
		flags.add(foDontVerifyChecksum);
	}
	if(version >= INNO_VERSION(5, 0, 3)) {
		flags.add(foUninsNoSharedFilePrompt);
	}
	if(version >= INNO_VERSION(5, 1, 0)) {
		flags.add(foCreateAllSubDirs);
	}
	if(version >= INNO_VERSION(5, 1, 2)) {
		flags.add(fo32Bit);
		flags.add(fo64Bit);
	}
	if(version >= INNO_VERSION(5, 2, 0)) {
		flags.add(foExternalSizePreset);
		flags.add(foSetNTFSCompression);
		flags.add(foUnsetNTFSCompression);
	}
	if(version >= INNO_VERSION(5, 2, 5)) {
		flags.add(foGacInstall);
	}
	
	options = flags.get();
	
	if(version.bits == 16 || version >= INNO_VERSION(5, 0, 0)) {
		type = StoredEnum<StoredFileType0>(is).get();
	} else {
		type = StoredEnum<StoredFileType1>(is).get();
	}
}

ENUM_NAMES(FileOptions::Enum, "File Option",
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
