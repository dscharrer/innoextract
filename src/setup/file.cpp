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

// Inno Setup 6.5.0 introduced the per-file verification kind alongside
// the SHA-256 hash and ISSig allowed-keys list appended to TSetupFileEntry.
STORED_ENUM_MAP(stored_file_verification_type, file_entry::FileVerificationNone,
	file_entry::FileVerificationNone,
	file_entry::FileVerificationHash,
	file_entry::FileVerificationISSig,
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

NAMES(setup::file_entry::file_verification_type, "File Verification Type",
	"none",
	"hash",
	"IS sig",
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
	
	if(i.version >= INNO_VERSION(6, 5, 0)) {
		// Inno Setup 6.5.0 appended five new expression strings (Excludes,
		// DownloadISSigSource, DownloadUserName, DownloadPassword,
		// ExtractArchivePassword) and a per-file Verification packed
		// record (ISSigAllowedKeys ansi string, 32-byte SHA-256 digest,
		// TSetupFileVerificationType byte enum) to TSetupFileEntry, after
		// the BeforeInstall string and before MinVersion. See
		// Projects/Src/Shared.Struct.pas at tag is-6_5_0 in
		// https://github.com/jrsoftware/issrc.
		is >> util::encoded_string(excludes, i.codepage, i.header.lead_bytes);
		is >> util::encoded_string(download_source, i.codepage, i.header.lead_bytes);
		is >> util::encoded_string(download_user, i.codepage, i.header.lead_bytes);
		is >> util::encoded_string(download_password, i.codepage, i.header.lead_bytes);
		is >> util::encoded_string(archive_password, i.codepage, i.header.lead_bytes);
		is >> util::ansi_string(issig_allowed_keys);
		is.read(checksum.sha256, std::streamsize(sizeof(checksum.sha256)));
		checksum.type = crypto::SHA256;
		verification = stored_enum<stored_file_verification_type>(is).get();
	} else {
		excludes.clear();
		download_source.clear();
		download_user.clear();
		download_password.clear();
		archive_password.clear();
		issig_allowed_keys.clear();
		verification = FileVerificationNone;
	}
	
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
	
	if(i.version >= INNO_VERSION_EXT(7, 0, 0, 3)) {
		// Inno Setup 7.0.0 (SetupID "7.0.0.3") removed the fo32Bit/fo64Bit
		// flags from TSetupFileEntryOption and moved the target
		// architecture into a dedicated TSetupEntryBitness byte enum
		// (ebInstallDefault, eb32Bit, eb64Bit, ebNativeBit,
		// ebCurrentProcessBit) stored before Options. See
		// Projects/Src/Shared.Struct.pas at tag is-7_0_0 in
		// https://github.com/jrsoftware/issrc. Translate the two
		// fixed-architecture values back into the existing Bits32/Bits64
		// flags so architecture-specific collision handling keeps working;
		// the runtime-resolved values are treated as architecture-neutral.
		boost::uint8_t bitness = util::load<boost::uint8_t>(is);
		if(bitness == 1) {
			options |= Bits32;
		} else if(bitness == 2) {
			options |= Bits64;
		}
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
	if(i.version >= INNO_VERSION(5, 1, 2) && i.version < INNO_VERSION_EXT(7, 0, 0, 3)) {
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
	if(i.version >= INNO_VERSION(6, 5, 0)) {
		// Inno Setup 6.5.0 added two flags at the end of the bitset for
		// the new [Files] flags `download` and `extractarchive`
		// (TSetupFileEntry.Options in Projects/Src/Shared.Struct.pas at
		// tag is-6_5_0 in https://github.com/jrsoftware/issrc).
		flagreader.add(Download);
		flagreader.add(ExtractArchive);
	}
	
	options |= flagreader.finalize();
	if(i.version >= INNO_VERSION(6, 7, 0)) {
		// Inno Setup 6.7.0 padded TSetupFileEntryOption to 57 elements
		// (foUnusedPadding=56) so the set is always 8 bytes regardless
		// of the actual flag count, matching the analogous change to
		// TSetupHeaderOption (see is-6_7_0 in issrc). Skip past the
		// extra padding bytes so the trailing FileType byte is read
		// from the right offset.
		flagreader.discard_padding_to(8);
	}
	
	if(i.version.bits() == 16 || i.version >= INNO_VERSION(5, 0, 0)) {
		type = stored_enum<stored_file_type_0>(is).get();
	} else {
		type = stored_enum<stored_file_type_1>(is).get();
	}
	
	additional_locations.clear();
	if(i.version < INNO_VERSION(6, 5, 0)) {
		// For Inno Setup 6.5.0+, file_entry::checksum carries the SHA-256
		// from the per-file ISSig Verification block, populated above.
		checksum.type = crypto::None;
	}
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
	"download",
	"extract archive",
	"readme",
)

NAMES(setup::file_entry::file_type, "File Entry Type",
	"user file",
	"uninstaller exe",
	"reg server exe",
)
