
#ifndef INNOEXTRACT_SETUP_FILEENTRY_HPP
#define INNOEXTRACT_SETUP_FILEENTRY_HPP

#include <stdint.h>
#include <string>
#include <iosfwd>

#include "setup/SetupItem.hpp"
#include "setup/Version.hpp"
#include "util/Enum.hpp"
#include "util/Flags.hpp"

struct FileEntry : public SetupItem {
	
	FLAGS(Options,
		
		ConfirmOverwrite,
		NeverUninstall,
		RestartReplace,
		DeleteAfterInstall,
		RegisterServer,
		RegisterTypeLib,
		SharedFile,
		CompareTimeStamp,
		FontIsNotTrueType,
		SkipIfSourceDoesntExist,
		OverwriteReadOnly,
		OverwriteSameVersion,
		CustomDestName,
		OnlyIfDestFileExists,
		NoRegError,
		UninsRestartDelete,
		OnlyIfDoesntExist,
		IgnoreVersion,
		PromptIfOlder,
		DontCopy,
		UninsRemoveReadOnly,
		RecurseSubDirsExternal,
		ReplaceSameVersionIfContentsDiffer,
		DontVerifyChecksum,
		UninsNoSharedFilePrompt,
		CreateAllSubDirs,
		Bits32,
		Bits64,
		ExternalSizePreset,
		SetNtfsCompression,
		UnsetNtfsCompression,
		GacInstall,
		
		// obsolete options:
		IsReadmeFile
	);
	
	enum Type {
		UserFile,
		UninstExe,
		RegSvrExe,
	};
	
	std::string source;
	std::string destination;
	std::string installFontName;
	std::string strongAssemblyName;
	
	int location; //!< index into the file location entry list
	uint32_t attributes;
	uint64_t externalSize;
	
	int permission; //!< index into the permission entry list
	
	Options options;
	
	Type type;
	
	void load(std::istream & is, const InnoVersion & version);
	
};

FLAGS_OVERLOADS(FileEntry::Options)
NAMED_ENUM(FileEntry::Options)

NAMED_ENUM(FileEntry::Type)

#endif // INNOEXTRACT_SETUP_FILEENTRY_HPP
