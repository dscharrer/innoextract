
#include "SetupHeader.hpp"

#include <cstdio>
#include <cstring>

#include <boost/static_assert.hpp>

#include "setup/SetupHeaderFormat.hpp"
#include "util/LoadingUtils.hpp"
#include "util/Utils.hpp"

void SetupHeader::load(std::istream & is, const InnoVersion & version) {
	
	options = 0;
	
	if(version < INNO_VERSION(1, 3, 21)) {
		::load<u32>(is); // uncompressed size of the setup header structure
	}
	
	is >> EncodedString(appName, version.codepage());
	is >> EncodedString(appVerName, version.codepage());
	if(version >= INNO_VERSION(1, 3, 21)) {
		is >> EncodedString(appId, version.codepage());
	}
	is >> EncodedString(appCopyright, version.codepage());
	if(version >= INNO_VERSION(1, 3, 21)) {
		is >> EncodedString(appPublisher, version.codepage());
		is >> EncodedString(appPublisherURL, version.codepage());
	} else {
		appPublisher.clear(), appPublisherURL.clear();
	}
	if(version >= INNO_VERSION(5, 1, 13)) {
		is >> EncodedString(appSupportPhone, version.codepage());
	} else {
		appSupportPhone.clear();
	}
	if(version >= INNO_VERSION(1, 3, 21)) {
		is >> EncodedString(appSupportURL, version.codepage());
		is >> EncodedString(appUpdatesURL, version.codepage());
		is >> EncodedString(appVersion, version.codepage());
	} else {
		appSupportURL.clear(), appUpdatesURL.clear(), appVersion.clear();
	}
	is >> EncodedString(defaultDirName, version.codepage());
	is >> EncodedString(defaultGroupName, version.codepage());
	if(version < INNO_VERSION(3, 0, 0)) {
		is >> AnsiString(uninstallIconName);
	} else {
		uninstallIconName.clear();
	}
	is >> EncodedString(baseFilename, version.codepage());
	if(version >= INNO_VERSION(1, 3, 21)) {
		if(version < INNO_VERSION(5, 2, 5)) {
			is >> AnsiString(licenseText);
			is >> AnsiString(infoBeforeText);
			is >> AnsiString(infoAfterText);
		}
		is >> EncodedString(uninstallFilesDir, version.codepage());
		is >> EncodedString(uninstallDisplayName, version.codepage());
		is >> EncodedString(uninstallDisplayIcon, version.codepage());
		is >> EncodedString(appMutex, version.codepage());
	} else {
		licenseText.clear(), infoBeforeText.clear(), infoAfterText.clear();
		uninstallFilesDir.clear(), uninstallDisplayName.clear();
		uninstallDisplayIcon.clear(), appMutex.clear();
	}
	if(version >= INNO_VERSION(3, 0, 0)) {
		is >> EncodedString(defaultUserInfoName, version.codepage());
		is >> EncodedString(defaultUserInfoOrg, version.codepage());
	} else {
		defaultUserInfoName.clear(), defaultUserInfoOrg.clear();
	}
	if(version >= INNO_VERSION_EXT(3, 0, 6, 1)) {
		is >> EncodedString(defaultUserInfoSerial, version.codepage());
		if(version < INNO_VERSION(5, 2, 5)) {
			is >> BinaryString(compiledCodeText);
		}
	} else {
		defaultUserInfoSerial.clear(), compiledCodeText.clear();
	}
	if(version >= INNO_VERSION(4, 2, 4)) {
		is >> EncodedString(appReadmeFile, version.codepage());
		is >> EncodedString(appContact, version.codepage());
		is >> EncodedString(appComments, version.codepage());
		is >> EncodedString(appModifyPath, version.codepage());
	} else {
		appReadmeFile.clear(), appContact.clear();
		appComments.clear(), appModifyPath.clear();
	}
	if(version >= INNO_VERSION(5, 3, 8)) {
		is >> EncodedString(createUninstallRegKey, version.codepage());
	} else {
		createUninstallRegKey.clear();
	}
	if(version >= INNO_VERSION(5, 3, 10)) {
		is >> EncodedString(uninstallable, version.codepage());
	} else {
		uninstallable.clear();
	}
	if(version >= INNO_VERSION(5, 2, 5)) {
		is >> AnsiString(licenseText);
		is >> AnsiString(infoBeforeText);
		is >> AnsiString(infoAfterText);
	}
	if(version >= INNO_VERSION(5, 2, 1) && version < INNO_VERSION(5, 3, 10)) {
		is >> BinaryString(signedUninstallerSignature);
	} else {
		signedUninstallerSignature.clear();
	}
	if(version >= INNO_VERSION(5, 2, 5)) {
		is >> BinaryString(compiledCodeText);
	}
	
	if(version >= INNO_VERSION(2, 0, 6) && !version.unicode) {
		leadBytes = CharSet(is).getBitSet();
	} else {
		leadBytes = 0;
	}
	
	if(version >= INNO_VERSION(4, 0, 0)) {
		numLanguageEntries = loadNumber<u32>(is);
	} else if(version >= INNO_VERSION(2, 0, 1)) {
		numLanguageEntries = 1;
	} else {
		numLanguageEntries = 0;
	}
	
	if(version >= INNO_VERSION(4, 2, 1)) {
		numCustomMessageEntries = loadNumber<u32>(is);
	} else {
		numCustomMessageEntries = 0;
	}
	
	if(version >= INNO_VERSION(4, 1, 0)) {
		numPermissionEntries = loadNumber<u32>(is);
	} else {
		numPermissionEntries = 0;
	}
	
	if(version >= INNO_VERSION(2, 0, 0)) {
		numTypeEntries = loadNumber<u32>(is);
		numComponentEntries = loadNumber<u32>(is);
		numTaskEntries = loadNumber<u32>(is);
	} else {
		numTypeEntries = 0, numComponentEntries = 0, numTaskEntries = 0;
	}
	
	numDirectoryEntries = loadNumber<u32>(is, version.bits);
	numFileEntries = loadNumber<u32>(is, version.bits);
	numFileLocationEntries = loadNumber<u32>(is, version.bits);
	numIconEntries = loadNumber<u32>(is, version.bits);
	numIniEntries = loadNumber<u32>(is, version.bits);
	numRegistryEntries = loadNumber<u32>(is, version.bits);
	numDeleteEntries = loadNumber<u32>(is, version.bits);
	numUninstallDeleteEntries = loadNumber<u32>(is, version.bits);
	numRunEntries = loadNumber<u32>(is, version.bits);
	numUninstallRunEntries = loadNumber<u32>(is, version.bits);
	
	size_t licenseSize;
	size_t infoBeforeSize;
	size_t infoAfterSize;
	if(version < INNO_VERSION(1, 3, 21)) {
		licenseSize = loadNumber<u32>(is, version.bits);
		infoBeforeSize = loadNumber<u32>(is, version.bits);
		infoAfterSize = loadNumber<u32>(is, version.bits);
	}
	
	minVersion.load(is, version);
	onlyBelowVersion.load(is, version);
	
	backColor = loadNumber<u32>(is);
	if(version >= INNO_VERSION(1, 3, 21)) {
		backColor2 = loadNumber<u32>(is);
	} else {
		backColor2 = 0;
	}
	wizardImageBackColor = loadNumber<u32>(is);
	if(version >= INNO_VERSION(2, 0, 0) && version < INNO_VERSION(5, 0, 4)) {
		wizardSmallImageBackColor = loadNumber<u32>(is);
	} else {
		wizardSmallImageBackColor = 0;
	}
	
	if(version < INNO_VERSION(4, 2, 0)) {
		password.crc32 = loadNumber<s32>(is), password.type = Checksum::Crc32;
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
		extraDiskSpaceRequired = loadNumber<s32>(is);
		slicesPerDisk = 1;
	} else {
		extraDiskSpaceRequired = loadNumber<s64>(is);
		slicesPerDisk = loadNumber<s32>(is);
	}
	
	if(version >= INNO_VERSION(2, 0, 0) && version < INNO_VERSION(5, 0, 0)) {
		installMode = StoredEnum<StoredInstallMode>(is).get();
	} else {
		installMode = NormalInstallMode;
	}
	
	if(version >= INNO_VERSION(1, 3, 21)) {
		uninstallLogMode = StoredEnum<StoredUninstallLogMode>(is).get();
	} else {
		uninstallLogMode = AppendLog;
	}
	
	if(version >= INNO_VERSION(2, 0, 0) && version < INNO_VERSION(5, 0, 0)) {
		uninstallStyle = StoredEnum<StoredUninstallStyle>(is).get();
	} else {
		uninstallStyle = (version < INNO_VERSION(5, 0, 0)) ? ClassicStyle : ModernStyle;
	}
	
	if(version >= INNO_VERSION(1, 3, 21)) {
		dirExistsWarning = StoredEnum<StoredDirExistsWarning>(is).get();
	} else {
		dirExistsWarning = Auto;
	}
	
	if(version >= INNO_VERSION(3, 0, 0) && version < INNO_VERSION(3, 0, 3)) {
		AutoBoolean val = StoredEnum<StoredRestartComputer>(is).get();
		switch(val) {
			case Yes: options |= AlwaysRestart; break;
			case Auto: options |= RestartIfNeededByRun; break;
			case No: break;
		}
	}
	
	if(version >= INNO_VERSION(5, 3, 7)) {
		privilegesRequired = StoredEnum<StoredPrivileges1>(is).get();
	} else if(version >= INNO_VERSION(3, 0, 4)) {
		privilegesRequired = StoredEnum<StoredPrivileges0>(is).get();
	}
	
	if(version >= INNO_VERSION(4, 0, 10)) {
		showLanguageDialog = StoredEnum<StoredShowLanguageDialog>(is).get();
		languageDetectionMethod = StoredEnum<StoredLanguageDetectionMethod>(is).get();
	}
	
	if(version >= INNO_VERSION(5, 3, 9)) {
		compressMethod = StoredEnum<StoredCompressionMethod3>(is).get();
	} else if(version >= INNO_VERSION(4, 2, 6)) {
		compressMethod = StoredEnum<StoredCompressionMethod2>(is).get();
	} else if(version >= INNO_VERSION(4, 2, 5)) {
		compressMethod = StoredEnum<StoredCompressionMethod1>(is).get();
	} else if(version >= INNO_VERSION(4, 1, 5)) {
		compressMethod = StoredEnum<StoredCompressionMethod0>(is).get();
	}
	
	if(version >= INNO_VERSION(5, 0, 2)) {
		architecturesAllowed = StoredFlags<StoredArchitectures>(is).get();
		architecturesInstallIn64BitMode = StoredFlags<StoredArchitectures>(is).get();
	} else {
		architecturesAllowed = Architectures::all();
		architecturesInstallIn64BitMode = Architectures::all();
	}
	
	if(version >= INNO_VERSION(5, 2, 1) && version < INNO_VERSION(5, 3, 10)) {
		signedUninstallerOrigSize = loadNumber<s32>(is);
		signedUninstallerHdrChecksum = loadNumber<u32>(is);
	} else {
		signedUninstallerOrigSize = signedUninstallerHdrChecksum = 0;
	}
	
	if(version >= INNO_VERSION(5, 3, 3)) {
		disableDirPage = StoredEnum<StoredDisablePage>(is).get();
		disableProgramGroupPage = StoredEnum<StoredDisablePage>(is).get();
	}
	
	if(version >= INNO_VERSION(5, 3, 6)) {
		uninstallDisplaySize = loadNumber<u32>(is);
	} else {
		uninstallDisplaySize = 0;
	}
	
	
	StoredFlagReader<Options> flags(is);
	
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
	
	options |= flags.get();
	
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
		if(licenseSize) {
			std::string temp;
			temp.resize(licenseSize);
			is.read(&temp[0], licenseSize);
			toUtf8(temp, licenseText);
		}
		if(infoBeforeSize) {
			std::string temp;
			temp.resize(infoBeforeSize);
			is.read(&temp[0], infoBeforeSize);
			toUtf8(temp, infoBeforeText);
		}
		if(infoAfterSize) {
			std::string temp;
			temp.resize(infoAfterSize);
			is.read(&temp[0], infoAfterSize);
			toUtf8(temp, infoAfterText);
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
BOOST_STATIC_ASSERT(EnumSize<SetupHeader::Options::Enum>::value == EnumNames<SetupHeader::Options::Enum>::count);

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
