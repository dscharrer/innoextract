
#ifndef INNOEXTRACT_SETUP_SETUPHEADERFORMAT_HPP
#define INNOEXTRACT_SETUP_SETUPHEADERFORMAT_HPP

#include "setup/SetupHeader.hpp"
#include "util/StoredEnum.hpp"

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

#endif // INNOEXTRACT_SETUP_SETUPHEADERFORMAT_HPP
