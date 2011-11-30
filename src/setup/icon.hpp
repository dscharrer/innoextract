
#ifndef INNOEXTRACT_SETUP_ICON_HPP
#define INNOEXTRACT_SETUP_ICON_HPP

#include <stdint.h>
#include <string>
#include <iosfwd>

#include "setup/item.hpp"
#include "util/enum.hpp"
#include "util/flags.hpp"

namespace setup {

struct version;

struct icon_entry : public item {
	
	FLAGS(flags,
		NeverUninstall,
		CreateOnlyIfFileExists,
		UseAppPaths,
		FolderShortcut,
		ExcludeFromShowInNewInstall,
		// obsolete options:
		RunMinimized
	);
	
	enum close_setting {
		NoSetting,
		CloseOnExit,
		DontCloseOnExit,
	};
	
	std::string name;
	std::string filename;
	std::string parameters;
	std::string working_dir;
	std::string icon_file;
	std::string comment;
	std::string app_user_model_id;
	
	int icon_index;
	
	int show_command;
	
	close_setting close_on_exit;
	
	uint16_t hotkey;
	
	flags options;
	
	void load(std::istream & is, const version & version);
	
};

} // namespace setup

NAMED_FLAGS(setup::icon_entry::flags)
NAMED_ENUM(setup::icon_entry::close_setting)

#endif // INNOEXTRACT_SETUP_ICON_HPP
