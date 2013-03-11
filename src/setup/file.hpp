/*
 * Copyright (C) 2011-2013 Daniel Scharrer
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

#ifndef INNOEXTRACT_SETUP_FILE_HPP
#define INNOEXTRACT_SETUP_FILE_HPP

#include <string>
#include <iosfwd>

#include <boost/cstdint.hpp>

#include "setup/item.hpp"
#include "util/enum.hpp"
#include "util/flags.hpp"

namespace setup {

struct version;

struct file_entry : public item {
	
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
	
	boost::uint32_t location; //!< index into the data entry list
	boost::uint32_t attributes;
	boost::uint64_t external_size;
	
	int permission; //!< index into the permission entry list
	
	flags options;
	
	file_type type;
	
	void load(std::istream & is, const version & version);
	
};

} // namespace setup

NAMED_FLAGS(setup::file_entry::flags)
NAMED_ENUM(setup::file_entry::file_type)

#endif // INNOEXTRACT_SETUP_FILE_HPP
