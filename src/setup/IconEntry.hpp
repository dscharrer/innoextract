
#ifndef INNOEXTRACT_SETUP_ICONENTRY_HPP
#define INNOEXTRACT_SETUP_ICONENTRY_HPP

#include <stdint.h>
#include <string>
#include <iosfwd>

#include "setup/SetupItem.hpp"
#include "setup/version.hpp"
#include "util/enum.hpp"
#include "util/flags.hpp"

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
	
	uint16_t hotkey;
	
	Options options;
	
	void load(std::istream & is, const inno_version & version);
	
};

FLAGS_OVERLOADS(IconEntry::Options)
NAMED_ENUM(IconEntry::Options)

NAMED_ENUM(IconEntry::CloseOnExit)

#endif // INNOEXTRACT_SETUP_ICONENTRY_HPP
