
#ifndef INNOEXTRACT_SETUP_SETUPHEADER_HPP
#define INNOEXTRACT_SETUP_SETUPHEADER_HPP

#include <stddef.h>
#include <bitset>
#include <string>
#include <iostream>

#include "setup/Version.hpp"
#include "setup/WindowsVersion.hpp"
#include "util/Enum.hpp"
#include "util/Flags.hpp"
#include "util/Types.hpp"

typedef char MD5Digest[16];
typedef char SHA1Digest[20];
typedef char SetupSalt[8];

FLAGS(SetupHeaderOptions,
			
	shDisableStartupPrompt,
	shCreateAppDir,
	shAllowNoIcons,
	shAlwaysRestart,
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
	shAllowUNCPath,
	shUserInfoPage,
	shUsePreviousUserInfo,
	shUninstallRestartComputer,
	shRestartIfNeededByRun,
	shShowTasksTreeLines,
	shAllowCancelDuringInstall,
	shWizardImageStretch,
	shAppendDefaultDirName,
	shAppendDefaultGroupName,
	shEncryptionUsed,
	shChangesEnvironment,
	shShowUndisplayableLanguages,
	shSetupLogging,
	shSignedUninstaller,
	shUsePreviousLanguage,
	shDisableWelcomePage,
	
	// Obsolete flags
	shUninstallable,
	shDisableDirPage,
	shDisableProgramGroupPage,
	shDisableAppendDir,
	shAdminPrivilegesRequired,
	shAlwaysCreateUninstallIcon,
	shCreateUninstallRegKey,
	shBzipUsed,
	shShowLanguageDialog,
	shDetectLanguageUsingLocale,
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
	size_t numDirectoryEntries;
	size_t numFileEntries;
	size_t numFileLocationEntries;
	size_t numIconEntries;
	size_t numIniEntries;
	size_t numRegistryEntries;
	size_t numInstallDeleteEntries;
	size_t numUninstallDeleteEntries;
	size_t numRunEntries;
	size_t numUninstallRunEntries;
	
	WindowsVersion minVersion;
	WindowsVersion onlyBelowVersion;
	
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

#endif // INNOEXTRACT_SETUP_SETUPHEADER_HPP
