
#ifndef INNOEXTRACT_SETUP_ICONENTRY_HPP
#define INNOEXTRACT_SETUP_ICONENTRY_HPP

#include <iostream>

#include "setup/SetupItem.hpp"
#include "setup/Version.hpp"
#include "util/Enum.hpp"
#include "util/Flags.hpp"
#include "util/Types.hpp"

struct IconEntry : public SetupItem {
	
	FLAGS(Options,
		NeverUninstall,
		CreateOnlyIfFileExists,
		UseAppPaths,
		FolderShortcut,
		ExcludeFromShowInNewInstall,
		// obsolete options:
		RunMinimized
	);
	
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
	std::string appUserModelId;
	
	int iconIndex;
	
	int showCmd;
	
	CloseOnExit closeOnExit;
	
	u16 hotkey;
	
	Options options;
	
	void load(std::istream & is, const InnoVersion & version);
	
};

FLAGS_OVERLOADS(IconEntry::Options)
NAMED_ENUM(IconEntry::Options)

NAMED_ENUM(IconEntry::CloseOnExit)

#endif // INNOEXTRACT_SETUP_ICONENTRY_HPP
