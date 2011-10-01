
#ifndef INNOEXTRACT_SETUP_FILEENTRY_HPP
#define INNOEXTRACT_SETUP_FILEENTRY_HPP

#include <iostream>

#include "setup/Version.hpp"
#include "setup/WindowsVersion.hpp"
#include "util/Enum.hpp"
#include "util/Flags.hpp"
#include "util/Types.hpp"

FLAGS(FileOptions,
	foConfirmOverwrite,
	foUninsNeverUninstall,
	foRestartReplace,
	foDeleteAfterInstall,
	foRegisterServer,
	foRegisterTypeLib,
	foSharedFile,
	foCompareTimeStamp,
	foFontIsntTrueType,
	foSkipIfSourceDoesntExist,
	foOverwriteReadOnly,
	foOverwriteSameVersion,
	foCustomDestName,
	foOnlyIfDestFileExists,
	foNoRegError,
	foUninsRestartDelete,
	foOnlyIfDoesntExist,
	foIgnoreVersion,
	foPromptIfOlder,
	foDontCopy,
	foUninsRemoveReadOnly,
	foRecurseSubDirsExternal,
	foReplaceSameVersionIfContentsDiffer,
	foDontVerifyChecksum,
	foUninsNoSharedFilePrompt,
	foCreateAllSubDirs,
	fo32Bit,
	fo64Bit,
	foExternalSizePreset,
	foSetNTFSCompression,
	foUnsetNTFSCompression,
	foGacInstall,
	
	// obsolete options:
	foIsReadmeFile,
)

NAMED_ENUM(FileOptions::Enum)

struct FileEntry {
	
	enum Type {
		UserFile,
		UninstExe,
		RegSvrExe,
	};
	
	std::string source;
	std::string destination;
	std::string installFontName;
	std::string strongAssemblyName;
	std::string components;
	std::string tasks;
	std::string languages;
	std::string check;
	std::string afterInstall;
	std::string beforeInstall;
	
	WindowsVersion minVersion;
	WindowsVersion onlyBelowVersion;
	
	int location; //!< index into the file location entry list
	u32 attributes;
	u64 externalSize;
	
	int permission; //!< index into the permission entry list
	
	FileOptions options;
	
	Type type;
	
	void load(std::istream & is, const InnoVersion & version);
	
};

NAMED_ENUM(FileEntry::Type)

#endif // INNOEXTRACT_SETUP_FILEENTRY_HPP
