
#include "SetupHeader.hpp"

#include <cstdio>
#include <cstring>

#include <boost/static_assert.hpp>

#include "setup/SetupHeaderFormat.hpp"
#include "util/LoadingUtils.hpp"
#include "util/Utils.hpp"

void SetupHeader::load(std::istream & is, const InnoVersion & version) {
	
	options = 0;
	
	if(version <= INNO_VERSION(1, 2, 16)) {
		::load<u32>(is); // uncompressed size of the setup header structure
	}
	
	is >> EncodedString(appName, version.codepage());
	is >> EncodedString(appVerName, version.codepage());
	if(version > INNO_VERSION(1, 2, 16)) { // not in 1.2.16, in 1.3.25
		is >> EncodedString(appId, version.codepage());
	}
	is >> EncodedString(appCopyright, version.codepage());
	if(version > INNO_VERSION(1, 2, 16)) { // not in 1.2.16, in 1.3.25
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
	if(version > INNO_VERSION(1, 2, 16)) { // not in 1.2.16, in 1.3.25
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
	if(version > INNO_VERSION(1, 2, 16)) { // not in 1.2.16, in 1.3.25
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
	if(version >= INNO_VERSION(4, 2, 4)) { // TODO 4.2.4 setup files incorrectly specify the version as 4.2.3
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
	
	if(version > INNO_VERSION(1, 3, 26) && !version.unicode) {
		leadBytes = CharSet(is).getBitSet();
	} else {
		leadBytes = 0;
	}
	
	if(version >= INNO_VERSION(4, 0, 0)) {
		numLanguageEntries = loadNumber<u32>(is);
	} else {
		numLanguageEntries = (version > INNO_VERSION(1, 3, 26)) ? 1 : 0;
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
	
	if(version > INNO_VERSION(1, 3, 26)) { // not in 1.3.26, in 2.0.8
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
	numInstallDeleteEntries = loadNumber<u32>(is, version.bits);
	numUninstallDeleteEntries = loadNumber<u32>(is, version.bits);
	numRunEntries = loadNumber<u32>(is, version.bits);
	numUninstallRunEntries = loadNumber<u32>(is, version.bits);
	
	if(version <= INNO_VERSION(1, 2, 16)) { // in 1.2.16, not in 1.3.25
		licenseSize = loadNumber<u32>(is, version.bits);
		infoBeforeSize = loadNumber<u32>(is, version.bits);
		infoAfterSize = loadNumber<u32>(is, version.bits);
	} else {
		licenseSize = infoBeforeSize = infoAfterSize = 0;
	}
	
	minVersion.load(is, version);
	onlyBelowVersion.load(is, version);
	
	backColor = loadNumber<u32>(is);
	if(version > INNO_VERSION(1, 2, 16)) { // not in 1.2.16, in 1.3.25
		backColor2 = loadNumber<u32>(is);
	} else {
		backColor2 = 0;
	}
	wizardImageBackColor = loadNumber<u32>(is);
	if(version > INNO_VERSION(1, 3, 26) && version < INNO_VERSION(5, 0, 4)) { // not in 1.3.26, in 2.0.8
		wizardSmallImageBackColor = loadNumber<u32>(is);
	} else {
		wizardSmallImageBackColor = 0;
	}
	
	if(version < INNO_VERSION(4, 2, 0)) {
		password = loadNumber<s32>(is), passwordType = PlainPassword;
	} else if(version < INNO_VERSION(5, 3, 9)) {
		is.read(passwordMd5, sizeof(passwordMd5)), passwordType = Md5Password;
	} else {
		is.read(passwordSha1, sizeof(passwordSha1)), passwordType = Sha1Password;
	}
	if(version >= INNO_VERSION(4, 2, 2)) {
		is.read(passwordSalt, sizeof(passwordSalt));
	} else {
		memset(passwordSalt, 0, sizeof(passwordSalt));
	}
	
	if(version < INNO_VERSION(4, 0, 0)) {
		extraDiskSpaceRequired = loadNumber<s32>(is);
		slicesPerDisk = 0;
	} else {
		extraDiskSpaceRequired = loadNumber<s64>(is);
		slicesPerDisk = loadNumber<s32>(is);
	}
	
	if(version > INNO_VERSION(1, 3, 26) && version < INNO_VERSION(5, 0, 0)) {
		// removed in 5.0.0, not in 1.2.10, not in 1.3.25
		installMode = StoredEnum<StoredInstallMode>(is).get();
	} else {
		installMode = NormalInstallMode;
	}
	
	if(version > INNO_VERSION(1, 2, 16)) { // not in 1.2.16, in 1.3.25
		uninstallLogMode = StoredEnum<StoredUninstallLogMode>(is).get();
	} else {
		uninstallLogMode = AppendLog;
	}
	
	if(version > INNO_VERSION(1, 3, 26) && version < INNO_VERSION(5, 0, 0)) {
		uninstallStyle = StoredEnum<StoredUninstallStyle>(is).get();
	} else {
		uninstallStyle = (version < INNO_VERSION(5, 0, 0)) ? ClassicStyle : ModernStyle;
	}
	
	if(version > INNO_VERSION(1, 2, 16)) { // not in 1.2.16, in 1.3.25
		dirExistsWarning = StoredEnum<StoredDirExistsWarning>(is).get();
	} else {
		dirExistsWarning = Auto;
	}
	
	if(version >= INNO_VERSION(3, 0, 0) && version < INNO_VERSION(3, 0, 3)) { // only in [3.0.0, 3.0.3)?
		AutoBoolean val = StoredEnum<StoredRestartComputer>(is).get();
		switch(val) {
			case Yes: options |= shAlwaysRestart; break;
			case Auto: options |= shRestartIfNeededByRun; break;
			case No: break;
		}
	}
	
	if(version >= INNO_VERSION(5, 3, 7)) {
		privilegesRequired = StoredEnum<StoredPrivileges1>(is).get();
	} else if(version >= INNO_VERSION(3, 0, 4)) { // TODO 3.0.4 setup files incorrectly specify the version as 3.0.3
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
	
	
	StoredFlagReader<SetupHeaderOptions> flags;
	
	flags.add(shDisableStartupPrompt);
	if(version < INNO_VERSION(5, 3, 10)) {
		flags.add(shUninstallable);
	}
	flags.add(shCreateAppDir);
	if(version < INNO_VERSION(5, 3, 3)) {
		flags.add(shDisableDirPage);
	}
	if(version <= INNO_VERSION(1, 2, 16)) {
		flags.add(shDisableDirExistsWarning); // only in 1.2.10, not in 1.3.25
	}
	if(version < INNO_VERSION(5, 3, 3)) {
		flags.add(shDisableProgramGroupPage);
	}
	flags.add(shAllowNoIcons);
	if(version < INNO_VERSION(3, 0, 0) || version >= INNO_VERSION(3, 0, 3)) {
		flags.add(shAlwaysRestart);
	}
	if(version <= INNO_VERSION(1, 2, 16)) {
		flags.add(shBackSolid); // only in 1.2.10, not in 1.3.25
	}
	flags.add(shAlwaysUsePersonalGroup);
	flags.add(shWindowVisible);
	flags.add(shWindowShowCaption);
	flags.add(shWindowResizable);
	flags.add(shWindowStartMaximized);
	flags.add(shEnableDirDoesntExistWarning);
	if(version < INNO_VERSION(4, 1, 2)) {
		flags.add(shDisableAppendDir);
	}
	flags.add(shPassword);
	flags.add(shAllowRootDirectory);
	flags.add(shDisableFinishedPage);
	
	if(version.bits != 16) {
		if(version < INNO_VERSION(3, 0, 4)) { // TODO 3.0.4 setup files incorrectly specify the version as 3.0.3
			flags.add(shAdminPrivilegesRequired);
		}
		if(version < INNO_VERSION(3, 0, 0)) {
			flags.add(shAlwaysCreateUninstallIcon);
		}
		if(version <= INNO_VERSION(1, 2, 16)) {
			flags.add(shOverwriteUninstRegEntries); // only in 1.2.10, win32-only); not in 1.3.25
		}
		flags.add(shChangesAssociations);
	}
	
	if(version > INNO_VERSION(1, 2, 16)) { // new after 1.2.16); in 1.3.25
		if(version < INNO_VERSION(5, 3, 8)) {
			flags.add(shCreateUninstallRegKey);
		}
		flags.add(shUsePreviousAppDir);
		flags.add(shBackColorHorizontal);
		flags.add(shUsePreviousGroup);
		flags.add(shUpdateUninstallLogAppName);
	}
	
	if(version > INNO_VERSION(1, 3, 26)) { // new after 1.3.26
		flags.add(shUsePreviousSetupType);
		flags.add(shDisableReadyMemo);
		flags.add(shAlwaysShowComponentsList);
		flags.add(shFlatComponentsList);
		flags.add(shShowComponentSizes);
		flags.add(shUsePreviousTasks);
		flags.add(shDisableReadyPage);
		flags.add(shAlwaysShowDirOnReadyPage);
		flags.add(shAlwaysShowGroupOnReadyPage);
	}
	
	if(version >= INNO_VERSION(2, 0, 17) && version < INNO_VERSION(4, 1, 5)) {
		flags.add(shBzipUsed);
	}
	
	if(version >= INNO_VERSION(2, 0, 18)) {
		flags.add(shAllowUNCPath);
	}
	
	if(version >= INNO_VERSION(3, 0, 0)) {
		flags.add(shUserInfoPage);
		flags.add(shUsePreviousUserInfo);
	}
	
	if(version >= INNO_VERSION(3, 0, 1)) {
		flags.add(shUninstallRestartComputer);
	}
	
	if(version >= INNO_VERSION(3, 0, 3)) {
		flags.add(shRestartIfNeededByRun);
	}
	
	if(version >= INNO_VERSION_EXT(3, 0, 6, 1)) {
		flags.add(shShowTasksTreeLines);
	}
	
	if(version >= INNO_VERSION(4, 0, 0) && version < INNO_VERSION(4, 0, 10)) {
		flags.add(shShowLanguageDialog);
	}
	
	if(version >= INNO_VERSION(4, 0, 1) && version < INNO_VERSION(4, 0, 10)) {
		flags.add(shDetectLanguageUsingLocale);
	}
	
	if(version >= INNO_VERSION(4, 0, 9)) {
		flags.add(shAllowCancelDuringInstall);
	}
	
	if(version >= INNO_VERSION(4, 1, 3)) {
		flags.add(shWizardImageStretch);
	}
	
	if(version >= INNO_VERSION(4, 1, 8)) {
		flags.add(shAppendDefaultDirName);
		flags.add(shAppendDefaultGroupName);
	}
	
	if(version >= INNO_VERSION(4, 2, 2)) {
		flags.add(shEncryptionUsed);
	}
	
	if(version >= INNO_VERSION(5, 0, 4)) {
		flags.add(shChangesEnvironment);
	}
	
	if(version >= INNO_VERSION(5, 1, 7) && !version.unicode) {
		flags.add(shShowUndisplayableLanguages);
	}
	
	if(version >= INNO_VERSION(5, 1, 13)) {
		flags.add(shSetupLogging);
	}
	
	if(version >= INNO_VERSION(5, 2, 1)) {
		flags.add(shSignedUninstaller);
	}
	
	if(version >= INNO_VERSION(5, 3, 8)) {
		flags.add(shUsePreviousLanguage);
	}
	
	if(version >= INNO_VERSION(5, 3, 9)) {
		flags.add(shDisableWelcomePage);
	}
	
	options |= flags.get(is);
	
	if(version < INNO_VERSION(3, 0, 4)) { // TODO 3.0.4 setup files incorrectly specify the version as 3.0.3
		privilegesRequired = (options & shAdminPrivilegesRequired) ? AdminPriviliges : NoPrivileges;
	}
	
	if(version < INNO_VERSION(4, 0, 10)) {
		showLanguageDialog = (options & shShowLanguageDialog) ? Yes : No;
		languageDetectionMethod = (options & shDetectLanguageUsingLocale) ? LocaleLanguage : UILanguage;
	}
	
	if(version < INNO_VERSION(4, 1, 5)) {
		compressMethod = (options & shBzipUsed) ? BZip2 : Zlib;
	}
	
	if(version < INNO_VERSION(5, 3, 3)) {
		disableDirPage = (options & shDisableDirPage) ? Yes : No;
		disableProgramGroupPage = (options & shDisableProgramGroupPage) ? Yes : No;
	}
	
}

ENUM_NAMES(SetupHeaderOptions::Enum, "Setup Option",
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
BOOST_STATIC_ASSERT(EnumSize<SetupHeaderOptions::Enum>::value == EnumNames<SetupHeaderOptions::Enum>::count);

ENUM_NAMES(Architectures::Enum, "Architecture",
	"unknown",
	"x86",
	"amd64",
	"IA64",
)

ENUM_NAMES(SetupHeader::PasswordType, "Password Type",
	"plain",
	"MD5",
	"SHA1",
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
