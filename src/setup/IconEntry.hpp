
#ifndef INNOEXTRACT_SETUP_ICONENTRY_HPP
#define INNOEXTRACT_SETUP_ICONENTRY_HPP

#include <iostream>

#include "setup/SetupCondition.hpp"
#include "setup/Version.hpp"
#include "setup/WindowsVersion.hpp"
#include "util/Enum.hpp"
#include "util/Flags.hpp"
#include "util/Types.hpp"

FLAGS(IconFlags,
	ioUninsNeverUninstall,
	ioCreateOnlyIfFileExists,
	ioUseAppPaths,
	ioFolderShortcut,
	ioExcludeFromShowInNewInstall,
	// obsolete options:
	ioRunMinimized,
)

NAMED_ENUM(IconFlags::Enum)

struct IconEntry {
	
	enum CloseOnExit {
		NoSetting,
		Yes,
		No,
	};
	
	std::string name;
	std::string filename;
	std::string parameters;
	std::string workingDir;
	std::string iconFilename;
	std::string comment;
	SetupCondition condition;
	SetupTasks tasks;
	std::string appUserModelId;
	
	WindowsVersion minVersion;
	WindowsVersion onlyBelowVersion;
	
	int iconIndex;
	
	int showCmd;
	
	CloseOnExit closeOnExit;
	
	u16 hotkey;
	
	IconFlags options;
	
	void load(std::istream & is, const InnoVersion & version);
	
};

NAMED_ENUM(IconEntry::CloseOnExit)

#endif // INNOEXTRACT_SETUP_ICONENTRY_HPP
