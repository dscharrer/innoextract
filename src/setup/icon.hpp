/*
 * Copyright (C) 2011-2020 Daniel Scharrer
 *
 * This software is provided 'as-is', without any express or implied
 * warranty.  In no event will the author(s) be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 */

/*!
 * \file
 *
 * Structures for menu/desktop shortcuts stored in Inno Setup files.
 */
#ifndef INNOEXTRACT_SETUP_ICON_HPP
#define INNOEXTRACT_SETUP_ICON_HPP

#include <string>
#include <iosfwd>

#include <boost/cstdint.hpp>

#include "setup/item.hpp"
#include "util/enum.hpp"
#include "util/flags.hpp"

namespace setup {

struct info;

struct icon_entry : public item {
	
	FLAGS(flags,
		NeverUninstall,
		CreateOnlyIfFileExists,
		UseAppPaths,
		FolderShortcut,
		ExcludeFromShowInNewInstall,
		PreventPinning,
		HasAppUserModelToastActivatorCLSID,
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
	std::string app_user_model_toast_activator_clsid;
	
	int icon_index;
	
	int show_command;
	
	close_setting close_on_exit;
	
	boost::uint16_t hotkey;
	
	flags options;
	
	void load(std::istream & is, const info & i);
	
};

} // namespace setup

NAMED_FLAGS(setup::icon_entry::flags)
NAMED_ENUM(setup::icon_entry::close_setting)

#endif // INNOEXTRACT_SETUP_ICON_HPP
