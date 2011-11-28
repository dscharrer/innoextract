
#ifndef INNOEXTRACT_SETUP_FILE_HPP
#define INNOEXTRACT_SETUP_FILE_HPP

#include <stdint.h>
#include <string>
#include <iosfwd>

#include "setup/SetupItem.hpp"
#include "setup/version.hpp"
#include "util/enum.hpp"
#include "util/flags.hpp"

namespace setup {

struct file_entry : public SetupItem {
	
	FLAGS(flags,
		
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
	
	enum file_type {
		UserFile,
		UninstExe,
		RegSvrExe,
	};
	
	std::string source;
	std::string destination;
	std::string install_font_name;
	std::string strong_assembly_name;
	
	uint32_t location; //!< index into the file location entry list
	uint32_t attributes;
	uint64_t external_size;
	
	int permission; //!< index into the permission entry list
	
	flags options;
	
	file_type type;
	
	void load(std::istream & is, const inno_version & version);
	
};

} // namespace setup

FLAGS_OVERLOADS(setup::file_entry::flags)
NAMED_ENUM(setup::file_entry::flags)

NAMED_ENUM(setup::file_entry::file_type)

#endif // INNOEXTRACT_SETUP_FILE_HPP
