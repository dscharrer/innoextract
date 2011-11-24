
#ifndef INNOEXTRACT_SETUP_SETUPHEADER_HPP
#define INNOEXTRACT_SETUP_SETUPHEADER_HPP

#include <stdint.h>
#include <stddef.h>
#include <bitset>
#include <string>
#include <iosfwd>

#include "crypto/checksum.hpp"
#include "setup/Version.hpp"
#include "setup/WindowsVersion.hpp"
#include "stream/chunk.hpp"
#include "util/enum.hpp"
#include "util/flags.hpp"

typedef char SetupSalt[8];

struct SetupHeader {
	
	// Setup data header.
	
	FLAGS(Options,
		
		DisableStartupPrompt,
		CreateAppDir,
		AllowNoIcons,
		AlwaysRestart,
		AlwaysUsePersonalGroup,
		WindowVisible,
		WindowShowCaption,
		WindowResizable,
		WindowStartMaximized,
		EnableDirDoesntExistWarning,
		Password,
		AllowRootDirectory,
		DisableFinishedPage,
		ChangesAssociations,
		UsePreviousAppDir,
		BackColorHorizontal,
		UsePreviousGroup,
		UpdateUninstallLogAppName,
		UsePreviousSetupType,
		DisableReadyMemo,
		AlwaysShowComponentsList,
		FlatComponentsList,
		ShowComponentSizes,
		UsePreviousTasks,
		DisableReadyPage,
		AlwaysShowDirOnReadyPage,
		AlwaysShowGroupOnReadyPage,
		AllowUNCPath,
		UserInfoPage,
		UsePreviousUserInfo,
		UninstallRestartComputer,
		RestartIfNeededByRun,
		ShowTasksTreeLines,
		AllowCancelDuringInstall,
		WizardImageStretch,
		AppendDefaultDirName,
		AppendDefaultGroupName,
		EncryptionUsed,
		ChangesEnvironment,
		ShowUndisplayableLanguages,
		SetupLogging,
		SignedUninstaller,
		UsePreviousLanguage,
		DisableWelcomePage,
		
		// Obsolete flags
		Uninstallable,
		DisableDirPage,
		DisableProgramGroupPage,
		DisableAppendDir,
		AdminPrivilegesRequired,
		AlwaysCreateUninstallIcon,
		CreateUninstallRegKey,
		BzipUsed,
		ShowLanguageDialog,
		DetectLanguageUsingLocale,
		DisableDirExistsWarning,
		BackSolid,
		OverwriteUninstRegEntries
	);
	
	FLAGS(Architectures,
		ArchitectureUnknown,
		X86,
		Amd64,
		IA64
	);
	
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
	size_t numDeleteEntries;
	size_t numUninstallDeleteEntries;
	size_t numRunEntries;
	size_t numUninstallRunEntries;
	
	WindowsVersion minVersion;
	WindowsVersion onlyBelowVersion;
	
	typedef uint32_t Color;
	Color backColor;
	Color backColor2;
	Color wizardImageBackColor;
	Color wizardSmallImageBackColor;
	
	crypto::checksum password;
	SetupSalt passwordSalt;
	
	int64_t extraDiskSpaceRequired;
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
	
	stream::chunk::compression_method compressMethod;
	
	Architectures architecturesAllowed;
	Architectures architecturesInstallIn64BitMode;
	
	uint32_t signedUninstallerOrigSize;
	uint32_t signedUninstallerHdrChecksum;
	
	AutoBoolean disableDirPage;
	AutoBoolean disableProgramGroupPage;
	
	size_t uninstallDisplaySize;
	
	Options options;
	
	void load(std::istream & is, const InnoVersion & version);
	
};

FLAGS_OVERLOADS(SetupHeader::Options)
NAMED_ENUM(SetupHeader::Options)

FLAGS_OVERLOADS(SetupHeader::Architectures)
NAMED_ENUM(SetupHeader::Architectures)

NAMED_ENUM(SetupHeader::InstallMode)

NAMED_ENUM(SetupHeader::UninstallLogMode)

NAMED_ENUM(SetupHeader::UninstallStyle)

NAMED_ENUM(SetupHeader::AutoBoolean)

NAMED_ENUM(SetupHeader::Privileges)

NAMED_ENUM(SetupHeader::LanguageDetection)

#endif // INNOEXTRACT_SETUP_SETUPHEADER_HPP
