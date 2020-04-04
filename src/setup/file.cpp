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

#include "setup/file.hpp"

#include "setup/info.hpp"
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

void file_entry::load(std::istream & is, const info & i) {
	
	USE_ENUM_NAMES(file_copy_mode)
	
	options = 0;
	
	if(i.version < INNO_VERSION(1, 3, 0)) {
		(void)util::load<boost::uint32_t>(is); // uncompressed size of the entry
	}
	
	is >> util::encoded_string(source, i.codepage, i.header.lead_bytes);
	is >> util::encoded_string(destination, i.codepage, i.header.lead_bytes);
	is >> util::encoded_string(install_font_name, i.codepage, i.header.lead_bytes);
	if(i.version >= INNO_VERSION(5, 2, 5)) {
		is >> util::encoded_string(strong_assembly_name, i.codepage, i.header.lead_bytes);
	} else {
		strong_assembly_name.clear();
	}
	
	load_condition_data(is, i);
	
	load_version_data(is, i.version);
	
	location = util::load<boost::uint32_t>(is, i.version.bits());
	attributes = util::load<boost::uint32_t>(is, i.version.bits());
	external_size = (i.version >= INNO_VERSION(4, 0, 0)) ? util::load<boost::uint64_t>(is)
	                                                     : util::load<boost::uint32_t>(is);
	
	if(i.version < INNO_VERSION(3, 0, 5)) {
		file_copy_mode copyMode = stored_enum<stored_file_copy_mode>(is).get();
		switch(copyMode) {
			case cmNormal: options |= PromptIfOlder; break;
			case cmIfDoesntExist: options |= OnlyIfDoesntExist | PromptIfOlder; break;
			case cmAlwaysOverwrite: options |= IgnoreVersion | PromptIfOlder; break;
			case cmAlwaysSkipIfSameOrOlder: break;
		}
	}
	
	if(i.version >= INNO_VERSION(4, 1, 0)) {
		permission = util::load<boost::int16_t>(is);
	} else {
		permission = boost::int16_t(-1);
	}
	
	stored_flag_reader<flags> flagreader(is, i.version.bits());
	
	flagreader.add(ConfirmOverwrite);
	flagreader.add(NeverUninstall);
	flagreader.add(RestartReplace);
	flagreader.add(DeleteAfterInstall);
	if(i.version.bits() != 16) {
		flagreader.add(RegisterServer);
		flagreader.add(RegisterTypeLib);
		flagreader.add(SharedFile);
	}
	if(i.version < INNO_VERSION(2, 0, 0) && !i.version.is_isx()) {
		flagreader.add(IsReadmeFile);
	}
	flagreader.add(CompareTimeStamp);
	flagreader.add(FontIsNotTrueType);
	if(i.version >= INNO_VERSION(1, 2, 5)) {
		flagreader.add(SkipIfSourceDoesntExist);
	}
	if(i.version >= INNO_VERSION(1, 2, 6)) {
		flagreader.add(OverwriteReadOnly);
	}
	if(i.version >= INNO_VERSION(1, 3, 21)) {
		flagreader.add(OverwriteSameVersion);
		flagreader.add(CustomDestName);
	}
	if(i.version >= INNO_VERSION(1, 3, 25)) {
		flagreader.add(OnlyIfDestFileExists);
	}
	if(i.version >= INNO_VERSION(2, 0, 5)) {
		flagreader.add(NoRegError);
	}
	if(i.version >= INNO_VERSION(3, 0, 1)) {
		flagreader.add(UninsRestartDelete);
	}
	if(i.version >= INNO_VERSION(3, 0, 5)) {
		flagreader.add(OnlyIfDoesntExist);
		flagreader.add(IgnoreVersion);
		flagreader.add(PromptIfOlder);
	}
	if(i.version >= INNO_VERSION(4, 0, 0) ||
	   (i.version.is_isx() && i.version >= INNO_VERSION_EXT(3, 0, 6, 1))) {
		flagreader.add(DontCopy);
	}
	if(i.version >= INNO_VERSION(4, 0, 5)) {
		flagreader.add(UninsRemoveReadOnly);
	}
	if(i.version >= INNO_VERSION(4, 1, 8)) {
		flagreader.add(RecurseSubDirsExternal);
	}
	if(i.version >= INNO_VERSION(4, 2, 1)) {
		flagreader.add(ReplaceSameVersionIfContentsDiffer);
	}
	if(i.version >= INNO_VERSION(4, 2, 5)) {
		flagreader.add(DontVerifyChecksum);
	}
	if(i.version >= INNO_VERSION(5, 0, 3)) {
		flagreader.add(UninsNoSharedFilePrompt);
	}
	if(i.version >= INNO_VERSION(5, 1, 0)) {
		flagreader.add(CreateAllSubDirs);
	}
	if(i.version >= INNO_VERSION(5, 1, 2)) {
		flagreader.add(Bits32);
		flagreader.add(Bits64);
	}
	if(i.version >= INNO_VERSION(5, 2, 0)) {
		flagreader.add(ExternalSizePreset);
		flagreader.add(SetNtfsCompression);
		flagreader.add(UnsetNtfsCompression);
	}
	if(i.version >= INNO_VERSION(5, 2, 5)) {
		flagreader.add(GacInstall);
	}
	
	options |= flagreader;
	
	if(i.version.bits() == 16 || i.version >= INNO_VERSION(5, 0, 0)) {
		type = stored_enum<stored_file_type_0>(is).get();
	} else {
		type = stored_enum<stored_file_type_1>(is).get();
	}
	
	additional_locations.clear();
	checksum.type = crypto::None;
	size = 0;
	
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
