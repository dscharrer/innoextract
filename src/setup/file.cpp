/*
 * Copyright (C) 2011 Daniel Scharrer
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

#include "setup/file.hpp"

#include "setup/version.hpp"
#include "util/load.hpp"
#include "util/log.hpp"
#include "util/storedenum.hpp"

namespace setup {

namespace {

enum file_copy_mode {
	cmNormal,
	cmIfDoesntExist,
	cmAlwaysOverwrite,
	cmAlwaysSkipIfSameOrOlder,
};

STORED_ENUM_MAP(stored_file_copy_mode, cmNormal,
	cmNormal,
	cmIfDoesntExist,
	cmAlwaysOverwrite,
	cmAlwaysSkipIfSameOrOlder,
);

STORED_ENUM_MAP(stored_file_type_0, file_entry::UserFile,
	file_entry::UserFile,
	file_entry::UninstExe,
);

// win32, before 5.0.0
STORED_ENUM_MAP(stored_file_type_1, file_entry::UserFile,
	file_entry::UserFile,
	file_entry::UninstExe,
	file_entry::RegSvrExe,
);

} // anonymous namespace

} // namespace setup

NAMED_ENUM(setup::file_copy_mode)

NAMES(setup::file_copy_mode, "File Copy Mode",
	"normal",
	"if doesn't exist",
	"always overwrite",
	"always skip if same or older",
)

namespace setup {

void file_entry::load(std::istream & is, const version & version) {
	
	(void)enum_names<file_copy_mode>::names;
	
	options = 0;
	
	if(version < INNO_VERSION(1, 3, 21)) {
		::load<uint32_t>(is); // uncompressed size of the file entry structure
	}
	
	is >> encoded_string(source, version.codepage());
	is >> encoded_string(destination, version.codepage());
	is >> encoded_string(install_font_name, version.codepage());
	if(version >= INNO_VERSION(5, 2, 5)) {
		is >> encoded_string(strong_assembly_name, version.codepage());
	} else {
		strong_assembly_name.clear();
	}
	
	load_condition_data(is, version);
	
	load_version_data(is, version);
	
	location = load_number<uint32_t>(is, version.bits);
	attributes = load_number<uint32_t>(is, version.bits);
	external_size = (version >= INNO_VERSION(4, 0, 0)) ? load_number<uint64_t>(is)
	                                                  : load_number<uint32_t>(is);
	
	if(version < INNO_VERSION(3, 0, 5)) {
		file_copy_mode copyMode = stored_enum<stored_file_copy_mode>(is).get();
		switch(copyMode) {
			case cmNormal: options |= PromptIfOlder; break;
			case cmIfDoesntExist: options |= OnlyIfDoesntExist | PromptIfOlder; break;
			case cmAlwaysOverwrite: options |= IgnoreVersion | PromptIfOlder; break;
			case cmAlwaysSkipIfSameOrOlder: break;
		}
	}
	
	if(version >= INNO_VERSION(4, 1, 0)) {
		permission = load_number<int16_t>(is);
	} else {
		permission = -1;
	}
	
	stored_flag_reader<flags> flags(is);
	
	flags.add(ConfirmOverwrite);
	flags.add(NeverUninstall);
	flags.add(RestartReplace);
	flags.add(DeleteAfterInstall);
	if(version.bits != 16) {
		flags.add(RegisterServer);
		flags.add(RegisterTypeLib);
		flags.add(SharedFile);
	}
	if(version < INNO_VERSION(2, 0, 0)) {
		flags.add(IsReadmeFile);
	}
	flags.add(CompareTimeStamp);
	flags.add(FontIsNotTrueType);
	flags.add(SkipIfSourceDoesntExist);
	flags.add(OverwriteReadOnly);
	if(version >= INNO_VERSION(1, 3, 21)) {
		flags.add(OverwriteSameVersion);
		flags.add(CustomDestName);
	}
	if(version >= INNO_VERSION(1, 3, 25)) {
		flags.add(OnlyIfDestFileExists);
	}
	if(version >= INNO_VERSION(2, 0, 5)) {
		flags.add(NoRegError);
	}
	if(version >= INNO_VERSION(3, 0, 1)) {
		flags.add(UninsRestartDelete);
	}
	if(version >= INNO_VERSION(3, 0, 5)) {
		flags.add(OnlyIfDoesntExist);
		flags.add(IgnoreVersion);
		flags.add(PromptIfOlder);
	}
	if(version >= INNO_VERSION(3, 0, 8)) {
		flags.add(DontCopy);
	}
	if(version >= INNO_VERSION(4, 0, 5)) {
		flags.add(UninsRemoveReadOnly);
	}
	if(version >= INNO_VERSION(4, 1, 8)) {
		flags.add(RecurseSubDirsExternal);
	}
	if(version >= INNO_VERSION(4, 2, 1)) {
		flags.add(ReplaceSameVersionIfContentsDiffer);
	}
	if(version >= INNO_VERSION(4, 2, 5)) {
		flags.add(DontVerifyChecksum);
	}
	if(version >= INNO_VERSION(5, 0, 3)) {
		flags.add(UninsNoSharedFilePrompt);
	}
	if(version >= INNO_VERSION(5, 1, 0)) {
		flags.add(CreateAllSubDirs);
	}
	if(version >= INNO_VERSION(5, 1, 2)) {
		flags.add(Bits32);
		flags.add(Bits64);
	}
	if(version >= INNO_VERSION(5, 2, 0)) {
		flags.add(ExternalSizePreset);
		flags.add(SetNtfsCompression);
		flags.add(UnsetNtfsCompression);
	}
	if(version >= INNO_VERSION(5, 2, 5)) {
		flags.add(GacInstall);
	}
	
	options = flags;
	
	if(version >= INNO_VERSION(3, 0, 5) && version < INNO_VERSION(5, 0, 3)) {
		// TODO find out where this byte comes from
		int byte = is.get();
		if(byte) {
			log_warning << "[file] unknown byte: " << byte;
		}
	}
	
	if(version.bits == 16 || version >= INNO_VERSION(5, 0, 0)) {
		type = stored_enum<stored_file_type_0>(is).get();
	} else {
		type = stored_enum<stored_file_type_1>(is).get();
	}
}

} // namespace setup

NAMES(setup::file_entry::flags, "File Option",
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

NAMES(setup::file_entry::file_type, "File Entry Type",
	"user file",
	"uninstaller exe",
	"reg server exe",
)
