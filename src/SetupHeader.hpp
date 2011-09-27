
#ifndef INNOEXTRACT_SETUPHEADER_HPP
#define INNOEXTRACT_SETUPHEADER_HPP

#include <stddef.h>
#include <bitset>
#include <string>
#include <iostream>
#include "Types.hpp"
#include "Flags.hpp"
#include "Enum.hpp"
#include "Version.hpp"

struct SetupVersionData {
	
	s32 winVersion, ntVersion; // Cardinal
	s16 ntServicePack; // Word
	
	void load(std::istream & is, const InnoVersion & version);
	
};

std::ostream & operator<<(std::ostream & os, const SetupVersionData & svd);

typedef char MD5Digest[16];
typedef char SHA1Digest[20];
typedef char SetupSalt[8];

FLAGS(SetupHeaderOptions,
	
	shDisableStartupPrompt,
	shCreateAppDir,
	shAllowNoIcons,
	shAlwaysRestart, // TODO missing in [3.0.0, 3.0.3)
	shAlwaysUsePersonalGroup,
	shWindowVisible,
	shWindowShowCaption,
	shWindowResizable,
	shWindowStartMaximized,
	shEnableDirDoesntExistWarning,
	shPassword,
	shAllowRootDirectory,
	shDisableFinishedPage,
	shChangesAssociations,
	shUsePreviousAppDir,
	shBackColorHorizontal,
	shUsePreviousGroup,
	shUpdateUninstallLogAppName,
	shUsePreviousSetupType,
	shDisableReadyMemo,
	shAlwaysShowComponentsList,
	shFlatComponentsList,
	shShowComponentSizes,
	shUsePreviousTasks,
	shDisableReadyPage,
	shAlwaysShowDirOnReadyPage,
	shAlwaysShowGroupOnReadyPage,
	
	// new in 2.0.18
	shAllowUNCPath,
	
	// new in 3.0.0
	shUserInfoPage,
	shUsePreviousUserInfo,
	
	// new in 3.0.1
	shUninstallRestartComputer,
	
	// new in 3.0.3
	shRestartIfNeededByRun,
	
	// new in 3.0.8
	shShowTasksTreeLines,
	
	// new in 4.0.9
	shAllowCancelDuringInstall,
	
	// new in 4.1.3
	shWizardImageStretch,
	
	// new in 4.1.8
	shAppendDefaultDirName,
	shAppendDefaultGroupName,
	
	// new in 4.2.2
	shEncryptionUsed,
	
	// new in 5.0.4
	shChangesEnvironment,
	
	// new in 5.1.7
	shShowUndisplayableLanguages, // TODO 5.2.5+: only if not unicode
	
	// new in 5.1.13
	shSetupLogging,
	
	// new in 5.2.1
	shSignedUninstaller,
	
	// new in 5.3.8
	shUsePreviousLanguage,
	
	// new in 5.3.9
	shDisableWelcomePage,
	
	// Obsolete flags
	shUninstallable, // TODO removed in 5.3.10
	shDisableDirPage, // TODO removed in 5.3.3
	shDisableProgramGroupPage, // TODO removed in 5.3.3
	shDisableAppendDir, // TODO removed in 4.1.2
	shAdminPrivilegesRequired, // TODO removed in 3.0.4
	shAlwaysCreateUninstallIcon, // TODO removed in 3.0.0
	shCreateUninstallRegKey, // TODO removed in 5.3.8
	shBzipUsed, // only in [2.0.17, 4.1.5)
	shShowLanguageDialog, // only in [4.0.0, 4.0.10)
	shDetectLanguageUsingLocale, // only in [4.0.1, 4.0.10)
	
	// only in very old versions:
	shDisableDirExistsWarning,
	shBackSolid,
	shOverwriteUninstRegEntries,
)

NAMED_ENUM(SetupHeaderOptions::Enum)

FLAGS(Architectures,
	ArchitectureUnknown,
	ArchitectureX86,
	ArchitectureAmd64,
	ArchitectureIA64,
)

NAMED_ENUM(Architectures::Enum)

struct SetupHeader {
	
	// Setup data header.
	
	std::string appName;
	std::string appVerName;
	std::string appId;
	std::string appCopyright;
	std::string appPublisher;
	std::string appPublisherURL;
	std::string appSupportPhone;
	std::string appSupportURL;
	std::string appUpdatesURL;
	std::string appVersion;
	std::string defaultDirName;
	std::string defaultGroupName;
	std::string uninstallIconName;
	std::string baseFilename;
	std::string uninstallFilesDir;
	std::string uninstallDisplayName;
	std::string uninstallDisplayIcon;
	std::string appMutex;
	std::string defaultUserInfoName;
	std::string defaultUserInfoOrg;
	std::string defaultUserInfoSerial;
	std::string appReadmeFile;
	std::string appContact;
	std::string appComments;
	std::string appModifyPath;
	std::string createUninstallRegKey;
	std::string uninstallable;
	std::string licenseText;
	std::string infoBeforeText;
	std::string infoAfterText;
	std::string signedUninstallerSignature;
	std::string compiledCodeText;
	
	std::bitset<256> leadBytes;
	
	size_t numLanguageEntries;
	size_t numCustomMessageEntries;
	size_t numPermissionEntries;
	size_t numTypeEntries;
	size_t numComponentEntries;
	size_t numTaskEntries;
	size_t numDirEntries;
	size_t numFileEntries;
	size_t numFileLocationEntries;
	size_t numIconEntries;
	size_t numIniEntries;
	size_t numRegistryEntries;
	size_t numInstallDeleteEntries;
	size_t numUninstallDeleteEntries;
	size_t numRunEntries;
	size_t numUninstallRunEntries;
	
	size_t licenseSize;
	size_t infoBeforeSize;
	size_t infoAfterSize;
	
	SetupVersionData minVersion;
	SetupVersionData onlyBelowVersion;
	
	Color backColor;
	Color backColor2;
	Color wizardImageBackColor;
	Color wizardSmallImageBackColor;
	
	enum PasswordType {
		PlainPassword,
		Md5Password,
		Sha1Password
	};
	union {
		s32 password; // probably CRC32
		MD5Digest passwordMd5;
		SHA1Digest passwordSha1;
	};
	PasswordType passwordType;
	SetupSalt passwordSalt; 
	
	s64 extraDiskSpaceRequired;
	size_t slicesPerDisk;
	
	enum InstallMode {
		NormalInstallMode,
		SilentInstallMode,
		VerySilentInstallMode,
	};
	InstallMode installMode;
	
	enum UninstallLogMode {
		AppendLog,
		NewLog,
		OverwriteLog
	};
	UninstallLogMode uninstallLogMode;
	
	enum UninstallStyle {
		ClassicStyle,
		ModernStyle
	};
	UninstallStyle uninstallStyle;
	
	enum AutoBoolean {
		Auto,
		No,
		Yes
	};
	
	AutoBoolean dirExistsWarning;
	
	enum Privileges {
		NoPrivileges,
		PowerUserPrivileges,
		AdminPriviliges,
		LowestPrivileges
	};
	Privileges privilegesRequired;
	
	AutoBoolean showLanguageDialog;
	
	enum LanguageDetection {
		UILanguage,
		LocaleLanguage,
		NoLanguageDetection
	};
	LanguageDetection languageDetectionMethod;
	
	enum CompressionMethod {
		Stored,
		Zlib,
		BZip2,
		LZMA1,
		LZMA2,
		Unknown
	};
	CompressionMethod compressMethod;
	
	Architectures architecturesAllowed;
	Architectures architecturesInstallIn64BitMode;
	
	u64 signedUninstallerOrigSize;
	u32 signedUninstallerHdrChecksum;
	
	AutoBoolean disableDirPage;
	AutoBoolean disableProgramGroupPage;
	
	size_t uninstallDisplaySize;
	
	SetupHeaderOptions options;
	
	void load(std::istream & is, const InnoVersion & version);
	
};

NAMED_ENUM(SetupHeader::PasswordType)

NAMED_ENUM(SetupHeader::InstallMode)

NAMED_ENUM(SetupHeader::UninstallLogMode)

NAMED_ENUM(SetupHeader::UninstallStyle)

NAMED_ENUM(SetupHeader::AutoBoolean)

NAMED_ENUM(SetupHeader::Privileges)

NAMED_ENUM(SetupHeader::LanguageDetection)

NAMED_ENUM(SetupHeader::CompressionMethod)

#endif // INNOEXTRACT_SETUPHEADER_HPP
