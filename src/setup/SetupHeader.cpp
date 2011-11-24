
#include "SetupHeader.hpp"

#include <cstdio>
#include <cstring>

#include <boost/static_assert.hpp>

#include "util/load.hpp"
#include "util/storedenum.hpp"

namespace {

STORED_ENUM_MAP(StoredInstallMode, SetupHeader::NormalInstallMode,
	SetupHeader::NormalInstallMode,
	SetupHeader::SilentInstallMode,
	SetupHeader::VerySilentInstallMode
);

STORED_ENUM_MAP(StoredUninstallLogMode, SetupHeader::AppendLog,
	SetupHeader::AppendLog,
	SetupHeader::NewLog,
	SetupHeader::OverwriteLog
);

STORED_ENUM_MAP(StoredUninstallStyle, SetupHeader::ClassicStyle,
	SetupHeader::ClassicStyle,
	SetupHeader::ModernStyle
);

STORED_ENUM_MAP(StoredDirExistsWarning, SetupHeader::Auto,
	SetupHeader::Auto,
	SetupHeader::No,
	SetupHeader::Yes
);

// pre- 5.3.7
STORED_ENUM_MAP(StoredPrivileges0, SetupHeader::NoPrivileges,
	SetupHeader::NoPrivileges,
	SetupHeader::PowerUserPrivileges,
	SetupHeader::AdminPriviliges,
);

// post- 5.3.7
STORED_ENUM_MAP(StoredPrivileges1, SetupHeader::NoPrivileges,
	SetupHeader::NoPrivileges,
	SetupHeader::PowerUserPrivileges,
	SetupHeader::AdminPriviliges,
	SetupHeader::LowestPrivileges
);

STORED_ENUM_MAP(StoredShowLanguageDialog, SetupHeader::Yes,
	SetupHeader::Yes,
	SetupHeader::No,
	SetupHeader::Auto
);

STORED_ENUM_MAP(StoredLanguageDetectionMethod, SetupHeader::UILanguage,
	SetupHeader::UILanguage,
	SetupHeader::LocaleLanguage,
	SetupHeader::NoLanguageDetection
);

STORED_FLAGS_MAP(StoredArchitectures,
	SetupHeader::ArchitectureUnknown,
	SetupHeader::X86,
	SetupHeader::Amd64,
	SetupHeader::IA64
);

STORED_ENUM_MAP(StoredRestartComputer, SetupHeader::Auto,
	SetupHeader::Auto,
	SetupHeader::No,
	SetupHeader::Yes
);

// pre-4.2.5
STORED_ENUM_MAP(StoredCompressionMethod0, SetupHeader::Unknown,
	SetupHeader::Zlib,
	SetupHeader::BZip2,
	SetupHeader::LZMA1
);

// 4.2.5
STORED_ENUM_MAP(StoredCompressionMethod1, SetupHeader::Unknown,
	SetupHeader::Stored,
	SetupHeader::BZip2,
	SetupHeader::LZMA1
);

// [4.2.6 5.3.9)
STORED_ENUM_MAP(StoredCompressionMethod2, SetupHeader::Unknown,
	SetupHeader::Stored,
	SetupHeader::Zlib,
	SetupHeader::BZip2,
	SetupHeader::LZMA1
);

// 5.3.9+
STORED_ENUM_MAP(StoredCompressionMethod3, SetupHeader::Unknown,
	SetupHeader::Stored,
	SetupHeader::Zlib,
	SetupHeader::BZip2,
	SetupHeader::LZMA1,
	SetupHeader::LZMA2
);

STORED_ENUM_MAP(StoredDisablePage, SetupHeader::Auto,
	SetupHeader::Auto,
	SetupHeader::No,
	SetupHeader::Yes
);

} // anonymous namespace

void SetupHeader::load(std::istream & is, const InnoVersion & version) {
	
	options = 0;
	
	if(version < INNO_VERSION(1, 3, 21)) {
		::load<uint32_t>(is); // uncompressed size of the setup header structure
	}
	
	is >> encoded_string(appName, version.codepage());
	is >> encoded_string(appVerName, version.codepage());
	if(version >= INNO_VERSION(1, 3, 21)) {
		is >> encoded_string(appId, version.codepage());
	}
	is >> encoded_string(appCopyright, version.codepage());
	if(version >= INNO_VERSION(1, 3, 21)) {
		is >> encoded_string(appPublisher, version.codepage());
		is >> encoded_string(appPublisherURL, version.codepage());
	} else {
		appPublisher.clear(), appPublisherURL.clear();
	}
	if(version >= INNO_VERSION(5, 1, 13)) {
		is >> encoded_string(appSupportPhone, version.codepage());
	} else {
		appSupportPhone.clear();
	}
	if(version >= INNO_VERSION(1, 3, 21)) {
		is >> encoded_string(appSupportURL, version.codepage());
		is >> encoded_string(appUpdatesURL, version.codepage());
		is >> encoded_string(appVersion, version.codepage());
	} else {
		appSupportURL.clear(), appUpdatesURL.clear(), appVersion.clear();
	}
	is >> encoded_string(defaultDirName, version.codepage());
	is >> encoded_string(defaultGroupName, version.codepage());
	if(version < INNO_VERSION(3, 0, 0)) {
		is >> ansi_string(uninstallIconName);
	} else {
		uninstallIconName.clear();
	}
	is >> encoded_string(baseFilename, version.codepage());
	if(version >= INNO_VERSION(1, 3, 21)) {
		if(version < INNO_VERSION(5, 2, 5)) {
			is >> ansi_string(licenseText);
			is >> ansi_string(infoBeforeText);
			is >> ansi_string(infoAfterText);
		}
		is >> encoded_string(uninstallFilesDir, version.codepage());
		is >> encoded_string(uninstallDisplayName, version.codepage());
		is >> encoded_string(uninstallDisplayIcon, version.codepage());
		is >> encoded_string(appMutex, version.codepage());
	} else {
		licenseText.clear(), infoBeforeText.clear(), infoAfterText.clear();
		uninstallFilesDir.clear(), uninstallDisplayName.clear();
		uninstallDisplayIcon.clear(), appMutex.clear();
	}
	if(version >= INNO_VERSION(3, 0, 0)) {
		is >> encoded_string(defaultUserInfoName, version.codepage());
		is >> encoded_string(defaultUserInfoOrg, version.codepage());
	} else {
		defaultUserInfoName.clear(), defaultUserInfoOrg.clear();
	}
	if(version >= INNO_VERSION_EXT(3, 0, 6, 1)) {
		is >> encoded_string(defaultUserInfoSerial, version.codepage());
		if(version < INNO_VERSION(5, 2, 5)) {
			is >> binary_string(compiledCodeText);
		}
	} else {
		defaultUserInfoSerial.clear(), compiledCodeText.clear();
	}
	if(version >= INNO_VERSION(4, 2, 4)) {
		is >> encoded_string(appReadmeFile, version.codepage());
		is >> encoded_string(appContact, version.codepage());
		is >> encoded_string(appComments, version.codepage());
		is >> encoded_string(appModifyPath, version.codepage());
	} else {
		appReadmeFile.clear(), appContact.clear();
		appComments.clear(), appModifyPath.clear();
	}
	if(version >= INNO_VERSION(5, 3, 8)) {
		is >> encoded_string(createUninstallRegKey, version.codepage());
	} else {
		createUninstallRegKey.clear();
	}
	if(version >= INNO_VERSION(5, 3, 10)) {
		is >> encoded_string(uninstallable, version.codepage());
	} else {
		uninstallable.clear();
	}
	if(version >= INNO_VERSION(5, 2, 5)) {
		is >> ansi_string(licenseText);
		is >> ansi_string(infoBeforeText);
		is >> ansi_string(infoAfterText);
	}
	if(version >= INNO_VERSION(5, 2, 1) && version < INNO_VERSION(5, 3, 10)) {
		is >> binary_string(signedUninstallerSignature);
	} else {
		signedUninstallerSignature.clear();
	}
	if(version >= INNO_VERSION(5, 2, 5)) {
		is >> binary_string(compiledCodeText);
	}
	
	if(version >= INNO_VERSION(2, 0, 6) && !version.unicode) {
		leadBytes = stored_char_set(is);
	} else {
		leadBytes = 0;
	}
	
	if(version >= INNO_VERSION(4, 0, 0)) {
		numLanguageEntries = load_number<uint32_t>(is);
	} else if(version >= INNO_VERSION(2, 0, 1)) {
		numLanguageEntries = 1;
	} else {
		numLanguageEntries = 0;
	}
	
	if(version >= INNO_VERSION(4, 2, 1)) {
		numCustomMessageEntries = load_number<uint32_t>(is);
	} else {
		numCustomMessageEntries = 0;
	}
	
	if(version >= INNO_VERSION(4, 1, 0)) {
		numPermissionEntries = load_number<uint32_t>(is);
	} else {
		numPermissionEntries = 0;
	}
	
	if(version >= INNO_VERSION(2, 0, 0)) {
		numTypeEntries = load_number<uint32_t>(is);
		numComponentEntries = load_number<uint32_t>(is);
		numTaskEntries = load_number<uint32_t>(is);
	} else {
		numTypeEntries = 0, numComponentEntries = 0, numTaskEntries = 0;
	}
	
	numDirectoryEntries = load_number<uint32_t>(is, version.bits);
	numFileEntries = load_number<uint32_t>(is, version.bits);
	numFileLocationEntries = load_number<uint32_t>(is, version.bits);
	numIconEntries = load_number<uint32_t>(is, version.bits);
	numIniEntries = load_number<uint32_t>(is, version.bits);
	numRegistryEntries = load_number<uint32_t>(is, version.bits);
	numDeleteEntries = load_number<uint32_t>(is, version.bits);
	numUninstallDeleteEntries = load_number<uint32_t>(is, version.bits);
	numRunEntries = load_number<uint32_t>(is, version.bits);
	numUninstallRunEntries = load_number<uint32_t>(is, version.bits);
	
	int32_t licenseSize;
	int32_t infoBeforeSize;
	int32_t infoAfterSize;
	if(version < INNO_VERSION(1, 3, 21)) {
		licenseSize = load_number<int32_t>(is, version.bits);
		infoBeforeSize = load_number<int32_t>(is, version.bits);
		infoAfterSize = load_number<int32_t>(is, version.bits);
	}
	
	minVersion.load(is, version);
	onlyBelowVersion.load(is, version);
	
	backColor = load_number<uint32_t>(is);
	if(version >= INNO_VERSION(1, 3, 21)) {
		backColor2 = load_number<uint32_t>(is);
	} else {
		backColor2 = 0;
	}
	wizardImageBackColor = load_number<uint32_t>(is);
	if(version >= INNO_VERSION(2, 0, 0) && version < INNO_VERSION(5, 0, 4)) {
		wizardSmallImageBackColor = load_number<uint32_t>(is);
	} else {
		wizardSmallImageBackColor = 0;
	}
	
	if(version < INNO_VERSION(4, 2, 0)) {
		password.crc32 = load_number<uint32_t>(is), password.type = Checksum::Crc32;
	} else if(version < INNO_VERSION(5, 3, 9)) {
		is.read(password.md5, sizeof(password.md5)), password.type = Checksum::MD5;
	} else {
		is.read(password.sha1, sizeof(password.sha1)), password.type = Checksum::Sha1;
	}
	if(version >= INNO_VERSION(4, 2, 2)) {
		is.read(passwordSalt, sizeof(passwordSalt));
	} else {
		memset(passwordSalt, 0, sizeof(passwordSalt));
	}
	
	if(version < INNO_VERSION(4, 0, 0)) {
		extraDiskSpaceRequired = load_number<int32_t>(is);
		slicesPerDisk = 1;
	} else {
		extraDiskSpaceRequired = load_number<int64_t>(is);
		slicesPerDisk = load_number<uint32_t>(is);
	}
	
	if(version >= INNO_VERSION(2, 0, 0) && version < INNO_VERSION(5, 0, 0)) {
		installMode = stored_enum<StoredInstallMode>(is).get();
	} else {
		installMode = NormalInstallMode;
	}
	
	if(version >= INNO_VERSION(1, 3, 21)) {
		uninstallLogMode = stored_enum<StoredUninstallLogMode>(is).get();
	} else {
		uninstallLogMode = AppendLog;
	}
	
	if(version >= INNO_VERSION(2, 0, 0) && version < INNO_VERSION(5, 0, 0)) {
		uninstallStyle = stored_enum<StoredUninstallStyle>(is).get();
	} else {
		uninstallStyle = (version < INNO_VERSION(5, 0, 0)) ? ClassicStyle : ModernStyle;
	}
	
	if(version >= INNO_VERSION(1, 3, 21)) {
		dirExistsWarning = stored_enum<StoredDirExistsWarning>(is).get();
	} else {
		dirExistsWarning = Auto;
	}
	
	if(version >= INNO_VERSION(3, 0, 0) && version < INNO_VERSION(3, 0, 3)) {
		AutoBoolean val = stored_enum<StoredRestartComputer>(is).get();
		switch(val) {
			case Yes: options |= AlwaysRestart; break;
			case Auto: options |= RestartIfNeededByRun; break;
			case No: break;
		}
	}
	
	if(version >= INNO_VERSION(5, 3, 7)) {
		privilegesRequired = stored_enum<StoredPrivileges1>(is).get();
	} else if(version >= INNO_VERSION(3, 0, 4)) {
		privilegesRequired = stored_enum<StoredPrivileges0>(is).get();
	}
	
	if(version >= INNO_VERSION(4, 0, 10)) {
		showLanguageDialog = stored_enum<StoredShowLanguageDialog>(is).get();
		languageDetectionMethod = stored_enum<StoredLanguageDetectionMethod>(is).get();
	}
	
	if(version >= INNO_VERSION(5, 3, 9)) {
		compressMethod = stored_enum<StoredCompressionMethod3>(is).get();
	} else if(version >= INNO_VERSION(4, 2, 6)) {
		compressMethod = stored_enum<StoredCompressionMethod2>(is).get();
	} else if(version >= INNO_VERSION(4, 2, 5)) {
		compressMethod = stored_enum<StoredCompressionMethod1>(is).get();
	} else if(version >= INNO_VERSION(4, 1, 5)) {
		compressMethod = stored_enum<StoredCompressionMethod0>(is).get();
	}
	
	if(version >= INNO_VERSION(5, 1, 0)) {
		architecturesAllowed = stored_flags<StoredArchitectures>(is).get();
		architecturesInstallIn64BitMode = stored_flags<StoredArchitectures>(is).get();
	} else {
		architecturesAllowed = Architectures::all();
		architecturesInstallIn64BitMode = Architectures::all();
	}
	
	if(version >= INNO_VERSION(5, 2, 1) && version < INNO_VERSION(5, 3, 10)) {
		signedUninstallerOrigSize = load_number<uint32_t>(is);
		signedUninstallerHdrChecksum = load_number<uint32_t>(is);
	} else {
		signedUninstallerOrigSize = signedUninstallerHdrChecksum = 0;
	}
	
	if(version >= INNO_VERSION(5, 3, 3)) {
		disableDirPage = stored_enum<StoredDisablePage>(is).get();
		disableProgramGroupPage = stored_enum<StoredDisablePage>(is).get();
	}
	
	if(version >= INNO_VERSION(5, 3, 6)) {
		uninstallDisplaySize = load_number<uint32_t>(is);
	} else {
		uninstallDisplaySize = 0;
	}
	
	
	stored_flag_reader<Options> flags(is);
	
	flags.add(DisableStartupPrompt);
	if(version < INNO_VERSION(5, 3, 10)) {
		flags.add(Uninstallable);
	}
	flags.add(CreateAppDir);
	if(version < INNO_VERSION(5, 3, 3)) {
		flags.add(DisableDirPage);
	}
	if(version < INNO_VERSION(1, 3, 21)) {
		flags.add(DisableDirExistsWarning);
	}
	if(version < INNO_VERSION(5, 3, 3)) {
		flags.add(DisableProgramGroupPage);
	}
	flags.add(AllowNoIcons);
	if(version < INNO_VERSION(3, 0, 0) || version >= INNO_VERSION(3, 0, 3)) {
		flags.add(AlwaysRestart);
	}
	if(version < INNO_VERSION(1, 3, 21)) {
		flags.add(BackSolid);
	}
	flags.add(AlwaysUsePersonalGroup);
	flags.add(WindowVisible);
	flags.add(WindowShowCaption);
	flags.add(WindowResizable);
	flags.add(WindowStartMaximized);
	flags.add(EnableDirDoesntExistWarning);
	if(version < INNO_VERSION(4, 1, 2)) {
		flags.add(DisableAppendDir);
	}
	flags.add(Password);
	flags.add(AllowRootDirectory);
	flags.add(DisableFinishedPage);
	if(version.bits != 16) {
		if(version < INNO_VERSION(3, 0, 4)) {
			flags.add(AdminPrivilegesRequired);
		}
		if(version < INNO_VERSION(3, 0, 0)) {
			flags.add(AlwaysCreateUninstallIcon);
		}
		if(version < INNO_VERSION(1, 3, 21)) {
			flags.add(OverwriteUninstRegEntries);
		}
		flags.add(ChangesAssociations);
	}
	if(version >= INNO_VERSION(1, 3, 21)) {
		if(version < INNO_VERSION(5, 3, 8)) {
			flags.add(CreateUninstallRegKey);
		}
		flags.add(UsePreviousAppDir);
		flags.add(BackColorHorizontal);
		flags.add(UsePreviousGroup);
		flags.add(UpdateUninstallLogAppName);
	}
	if(version >= INNO_VERSION(2, 0, 0)) {
		flags.add(UsePreviousSetupType);
		flags.add(DisableReadyMemo);
		flags.add(AlwaysShowComponentsList);
		flags.add(FlatComponentsList);
		flags.add(ShowComponentSizes);
		flags.add(UsePreviousTasks);
		flags.add(DisableReadyPage);
	}
	if(version >= INNO_VERSION(2, 0, 7)) {
		flags.add(AlwaysShowDirOnReadyPage);
		flags.add(AlwaysShowGroupOnReadyPage);
	}
	if(version >= INNO_VERSION(2, 0, 17) && version < INNO_VERSION(4, 1, 5)) {
		flags.add(BzipUsed);
	}
	if(version >= INNO_VERSION(2, 0, 18)) {
		flags.add(AllowUNCPath);
	}
	if(version >= INNO_VERSION(3, 0, 0)) {
		flags.add(UserInfoPage);
		flags.add(UsePreviousUserInfo);
	}
	if(version >= INNO_VERSION(3, 0, 1)) {
		flags.add(UninstallRestartComputer);
	}
	if(version >= INNO_VERSION(3, 0, 3)) {
		flags.add(RestartIfNeededByRun);
	}
	if(version >= INNO_VERSION_EXT(3, 0, 6, 1)) {
		flags.add(ShowTasksTreeLines);
	}
	if(version >= INNO_VERSION(4, 0, 0) && version < INNO_VERSION(4, 0, 10)) {
		flags.add(ShowLanguageDialog);
	}
	
	if(version >= INNO_VERSION(4, 0, 1) && version < INNO_VERSION(4, 0, 10)) {
		flags.add(DetectLanguageUsingLocale);
	}
	if(version >= INNO_VERSION(4, 0, 9)) {
		flags.add(AllowCancelDuringInstall);
	} else {
		options |= AllowCancelDuringInstall;
	}
	if(version >= INNO_VERSION(4, 1, 3)) {
		flags.add(WizardImageStretch);
	}
	if(version >= INNO_VERSION(4, 1, 8)) {
		flags.add(AppendDefaultDirName);
		flags.add(AppendDefaultGroupName);
	}
	if(version >= INNO_VERSION(4, 2, 2)) {
		flags.add(EncryptionUsed);
	}
	if(version >= INNO_VERSION(5, 0, 4)) {
		flags.add(ChangesEnvironment);
	}
	if(version >= INNO_VERSION(5, 1, 7) && !version.unicode) {
		flags.add(ShowUndisplayableLanguages);
	}
	if(version >= INNO_VERSION(5, 1, 13)) {
		flags.add(SetupLogging);
	}
	if(version >= INNO_VERSION(5, 2, 1)) {
		flags.add(SignedUninstaller);
	}
	if(version >= INNO_VERSION(5, 3, 8)) {
		flags.add(UsePreviousLanguage);
	}
	if(version >= INNO_VERSION(5, 3, 9)) {
		flags.add(DisableWelcomePage);
	}
	
	options |= flags;
	
	if(version < INNO_VERSION(3, 0, 4)) {
		privilegesRequired = (options & AdminPrivilegesRequired) ? AdminPriviliges : NoPrivileges;
	}
	
	if(version < INNO_VERSION(4, 0, 10)) {
		showLanguageDialog = (options & ShowLanguageDialog) ? Yes : No;
		languageDetectionMethod = (options & DetectLanguageUsingLocale) ? LocaleLanguage : UILanguage;
	}
	
	if(version < INNO_VERSION(4, 1, 5)) {
		compressMethod = (options & BzipUsed) ? BZip2 : Zlib;
	}
	
	if(version < INNO_VERSION(5, 3, 3)) {
		disableDirPage = (options & DisableDirPage) ? Yes : No;
		disableProgramGroupPage = (options & DisableProgramGroupPage) ? Yes : No;
	}
	
	if(version < INNO_VERSION(1, 3, 21)) {
		if(licenseSize > 0) {
			std::string temp;
			temp.resize(size_t(licenseSize));
			is.read(&temp[0], licenseSize);
			to_utf8(temp, licenseText);
		}
		if(infoBeforeSize > 0) {
			std::string temp;
			temp.resize(size_t(infoBeforeSize));
			is.read(&temp[0], infoBeforeSize);
			to_utf8(temp, infoBeforeText);
		}
		if(infoAfterSize > 0) {
			std::string temp;
			temp.resize(size_t(infoAfterSize));
			is.read(&temp[0], infoAfterSize);
			to_utf8(temp, infoAfterText);
		}
	}
	
}

ENUM_NAMES(SetupHeader::Options, "Setup Option",
	"disable startup prompt",
	"create app dir",
	"allow no icons",
	"always restart",
	"always use personal group",
	"window visible",
	"window show caption",
	"window resizable",
	"window start maximized",
	"enable dir doesn't exist warning",
	"password",
	"allow root directory",
	"disable finished page",
	"changes associations",
	"use previous app dir",
	"back color horizontal",
	"use previous group",
	"update uninstall log app name",
	"use previous setup type",
	"disable ready memo",
	"always show components list",
	"flat components list",
	"show component sizes",
	"use previous tasks",
	"disable ready page",
	"always show dir on ready page",
	"always show group on ready page",
	"allow unc path",
	"user info page",
	"use previous user info",
	"uninstall restart computer",
	"restart if needed by run",
	"show tasks tree lines",
	"allow cancel during install",
	"wizard image stretch",
	"append default dir name",
	"append default group name",
	"encrypted",
	"changes environment",
	"show undisplayable languages",
	"setup logging",
	"signed uninstaller",
	"use previous language",
	"disable welcome page",
	"uninstallable",
	"disable dir page",
	"disable program group page",
	"disable append dir",
	"admin privilegesrequired",
	"always create uninstall icon",
	"create uninstall reg key",
	"bzip used",
	"show language dialog",
	"detect language using locale",
	"disable dir exists warning",
	"back solid",
	"overwrite uninst reg entries",
)
BOOST_STATIC_ASSERT(SetupHeader::Options::bits == enum_names<SetupHeader::Options::Enum>::count);

ENUM_NAMES(SetupHeader::Architectures, "Architecture",
	"unknown",
	"x86",
	"amd64",
	"IA64",
)

ENUM_NAMES(SetupHeader::InstallMode, "Install Mode",
	"normal",
	"silent",
	"very silent",
)

ENUM_NAMES(SetupHeader::UninstallLogMode, "Uninstall Log Mode",
	"append",
	"new log",
	"overwrite",
)

ENUM_NAMES(SetupHeader::UninstallStyle, "Uninstall Style",
	"classic",
	"modern",
)

ENUM_NAMES(SetupHeader::AutoBoolean, "Auto Boolean",
	"auto",
	"no",
	"yes",
)

ENUM_NAMES(SetupHeader::Privileges, "Privileges",
	"none",
	"power user",
	"admin",
	"lowest",
)

ENUM_NAMES(SetupHeader::LanguageDetection, "Language Detection",
	"ui language",
	"locale",
	"none",
)

ENUM_NAMES(SetupHeader::CompressionMethod, "Compression Method",
	"stored",
	"zlib",
	"bzip2",
	"lzma1",
	"lzma2",
	"unknown",
)
