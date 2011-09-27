
#include <vector>
#include <boost/utility/enable_if.hpp>
#include <boost/static_assert.hpp>

#include "Types.h"
#include "SetupHeader.hpp"
#include "LoadingUtils.hpp"
#include "Enum.hpp"
#include "Output.hpp"

template <class Enum>
struct EnumValueMap {
	
	typedef Enum enum_type;
	typedef Enum flag_type;
	
};

#define STORED_ENUM_MAP(MapName, Default, ...) \
struct MapName : public EnumValueMap<typeof(Default)> { \
	static const flag_type default_value; \
	static const flag_type values[]; \
	static const size_t count; \
}; \
const MapName::flag_type MapName::default_value = Default; \
const MapName::flag_type MapName::values[] = { __VA_ARGS__ }; \
const size_t MapName::count = ARRAY_SIZE(MapName::values)

#define STORED_FLAGS_MAP(MapName, Flag0, ...) STORED_ENUM_MAP(MapName, Flag0, Flag0, ## __VA_ARGS__)

template <class Mapping>
struct StoredEnum {
	
	u32 value;
	
public:
	
	typedef Mapping mapping_type;
	typedef typename Mapping::enum_type enum_type;
	
	static const size_t size = Mapping::count;
	
	inline StoredEnum(std::istream & is) {
		value = loadNumber<u8>(is); // TODO use larger types for larger enums
	}
	
	enum_type get() {
		
		if(value < size) {
			return Mapping::values[value];
		}
		
		warning << "warning: unexpected " << EnumNames<enum_type>::name << " value: " << value;
		
		return Mapping::default_value;
	}
	
};

template <size_t Bits>
class StoredBitfield {
	
	typedef u8 base_type;
	
	static const size_t base_size = sizeof(base_type) * 8;
	static const size_t count = (Bits + (base_size - 1)) / base_size; // ceildiv
	
	base_type bits[count];
	
public:
	
	static const size_t size = Bits;
	
	inline StoredBitfield(std::istream & is) {
		for(size_t i = 0; i < count; i++) {
			bits[i] = loadNumber<base_type>(is);
		}
	}
	
	inline u64 getLowerBits() const {
		
		BOOST_STATIC_ASSERT(sizeof(u64) % sizeof(base_type) == 0);
		
		u64 result = 0;
		
		for(size_t i = 0; i < std::min(sizeof(u64) / sizeof(base_type), count); i++) {
			result |= (u64(bits[i]) << (i * base_size));
		}
		
		return result;
	}
	
	inline std::bitset<size> getBitSet() const {
		
		static const size_t ulong_size = sizeof(unsigned long) * 8;
		
		BOOST_STATIC_ASSERT(base_size % ulong_size == 0 || base_size < ulong_size);
		
		std::bitset<size> result(0);
		for(size_t i = 0; i < count; i++) {
			for(size_t j = 0; j < ceildiv(base_size, ulong_size); j++) {
				result |= std::bitset<size>(static_cast<unsigned long>(bits[i] >> (j * ulong_size)))
				          << ((i * base_size) + (j * ulong_size));
			}
		}
		return result;
	}
	
};

template <class Mapping>
class StoredFlags : private StoredBitfield<Mapping::count> {
	
public:
	
	typedef Mapping mapping_type;
	typedef typename Mapping::enum_type enum_type;
	typedef Flags<enum_type> flag_type;
	
	inline StoredFlags(std::istream & is) : StoredBitfield<Mapping::count>(is) { }
	
	flag_type get() {
		
		u64 bits = this->getLowerBits();
		flag_type result = 0;
		
		for(size_t i = 0; i < this->size; i++) {
			if(bits & (u64(1) << i)) {
				result |= Mapping::values[i];
				bits &= ~(u64(1) << i);
			}
		}
		
		if(bits) {
			warning << "unexpected " << EnumNames<enum_type>::name << " flags: " << std::hex << bits << std::dec;
		}
		
		return result;
	}
	
};

template <class Enum>
class StoredFlagReader {
	
public:
	
	typedef Enum enum_type;
	typedef Flags<enum_type> flag_type;
	
	std::vector<enum_type> mappings;
	
	void add(enum_type flag) {
		mappings.push_back(flag);
	}
	
	
	flag_type get(std::istream & is) {
		
		u64 bits = 0;
		
		/*
		if(mappings.size() <= 32) {
			bits = loadNumber<u32>(is);
		} else if(mappings.size() <= 256) {
			bits = loadNumber<u64>(is);
			for(size_t i = 1; i < 4; i++) {
				u64 temp = loadNumber<u64>(is);
				if(temp) {
					warning << "unexpected " << EnumNames<enum_type>::name << " flags: " << std::hex << bits << std::dec << " @ " << i;
				}
			}
		} else {
			error << "error reading " << EnumNames<enum_type>::name << ": too many flags: " << mappings.size();
			bits = 0;
		}*/
		
		typedef u8 stored_type;
		static const size_t stored_bits = sizeof(stored_type) * 8;
		for(size_t i = 0; i < ceildiv(mappings.size(), stored_bits); i++) {
			bits |= u64(load<stored_type>(is)) << (i * stored_bits);
		}
		
		std::cout << "read " << mappings.size() << " flags: " << std::hex << bits << std::dec << std::endl;
		
		flag_type result = 0;
		
		for(size_t i = 0; i < mappings.size(); i++) {
			if(bits & (u64(1) << i)) {
				result |= mappings[i];
				bits &= ~(u64(1) << i);
			}
		}
		
		if(bits) {
			warning << "unexpected " << EnumNames<enum_type>::name << " flags: " << std::hex << bits << std::dec;
		}
		
		return result;
	}
	
};

template <class Enum>
class StoredFlagReader<Flags<Enum> > : public StoredFlagReader<Enum> { };

/*
enum _SetupHeaderOption {
	
	shDisableStartupPrompt,
	shUninstallable, // TODO removed in 5.3.10
	shCreateAppDir,
	shDisableDirPage, // TODO removed in 5.3.3
	shDisableDirExistsWarning, // TODO only in 1.2.10, not in 1.3.25
	shDisableProgramGroupPage, // TODO removed in 5.3.3
	shAllowNoIcons,
	shAlwaysRestart, // TODO missing in [3.0.0, 3.0.3)
	shBackSolid, // TODO only in 1.2.10, not in 1.3.25
	shAlwaysUsePersonalGroup,
	shWindowVisible,
	shWindowShowCaption,
	shWindowResizable,
	shWindowStartMaximized,
	shEnableDirDoesntExistWarning,
	shDisableAppendDir, // TODO removed in 4.1.2
	shPassword,
	shAllowRootDirectory,
	shDisableFinishedPage,
	shAdminPrivilegesRequired, // TODO removed in 3.0.4, 1.2.10: win32-only
	shAlwaysCreateUninstallIcon, // TODO removed in 3.0.0, 1.2.10: win32-only
	shOverwriteUninstRegEntries, // TODO only in 1.2.10, win32-only, not in 1.3.25
	shChangesAssociations, // TODO 1.2.10: win32-only
	
	// new after 1.2.16, in 1.3.25
	shCreateUninstallRegKey, // TODO removed in 5.3.8
	shUsePreviousAppDir,
	shBackColorHorizontal,
	shUsePreviousGroup,
	shUpdateUninstallLogAppName,
	
	// new after 1.3.26
	shUsePreviousSetupType,
	shDisableReadyMemo,
	shAlwaysShowComponentsList,
	shFlatComponentsList,
	shShowComponentSizes,
	shUsePreviousTasks,
	shDisableReadyPage,
	shAlwaysShowDirOnReadyPage,
	shAlwaysShowGroupOnReadyPage,
	
	// only in [2.0.17, 4.1.5)
	shBzipUsed,
	
	// new in 2.0.18
	shAllowUNCPath,
	
	// new in 3.0.0
	shUserInfoPage,
	shUsePreviousUserInfo,
	
	// new in 3.0.1
	shUninstallRestartComputer,
	
	// new in 3.0.3
	shRestartIfNeededByRun,
	
	// new in 3.0.6.1
	shShowTasksTreeLines,
	
	// only in [4.0.0, 4.0.10)
	shShowLanguageDialog,
	
	// only in [4.0.1, 4.0.10)
	shDetectLanguageUsingLocale,
	
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
	
};*/

typedef StoredBitfield<256> CharSet;

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
	ArchitectureUnknown,
	ArchitectureX86,
	ArchitectureAmd64,
	ArchitectureIA64
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

/*
struct SetupHeader {
	
	union {
	
	struct {
	
	std::string AppName, AppVerName, AppId, AppCopyright; // String TODO 1.2.10: PChar
	std::string AppPublisher, AppPublisherURL; // String TODO not in 1.2.10
	std::string AppSupportPhone; // String TODO new in 5.1.13
	std::string AppSupportURL, AppUpdatesURL, AppVersion; // String TODO not in 1.2.10
	std::string DefaultDirName, DefaultGroupName; // String
	std::string UninstallIconName; // String TODO removed in 3.0.0
	std::string BaseFilename; //String
	std::string LicenseText, InfoBeforeText, InfoAfterText, UninstallFilesDir,
		UninstallDisplayName, UninstallDisplayIcon, AppMutex; // String TODO not in 1.2.10
	std::string DefaultUserInfoName, DefaultUserInfoOrg; // String TODO new in 3.0.0
	std::string DefaultUserInfoSerial, CompiledCodeText; // String TODO new in 3.0.6.1
	std::string AppReadmeFile, AppContact, AppComments, AppModifyPath; // String TODO new in 4.2.4
	std::string SignedUninstallerSignature; // String TODO new in 5.2.1
	
	};
	
	struct { // TODO 5.2.5+
	
	std::wstring AppName, AppVerName, AppId, AppCopyright, AppPublisher, AppPublisherURL,
		AppSupportPhone, AppSupportURL, AppUpdatesURL, AppVersion, DefaultDirName,
		DefaultGroupName, BaseFilename, UninstallFilesDir, UninstallDisplayName,
		UninstallDisplayIcon, AppMutex, DefaultUserInfoName, DefaultUserInfoOrg,
		DefaultUserInfoSerial, AppReadmeFile, AppContact, AppComments,
		AppModifyPath; // String / WideString
	
	std::string CreateUninstallRegKey; // String / WideString TODO new in 5.3.8
	std::string Uninstallable; // String / WideString TODO new in 5.3.10
	
	std::string LicenseText, InfoBeforeText, InfoAfterText; // AnsiString
	std::string SignedUninstallerSignature; // AnsiString TODO removed in 5.3.10
	std::string CompiledCodeText; // AnsiString
	
	};
	
	};
	
	CharSet LeadBytes; // set of Char TODO 5.2.5+: set of AnsiChar, only exists if not unicode
	
	s32 NumLanguageEntries; // Integer TODO new in 4.0.0
	s32 NumCustomMessageEntries; // Integer TODO new in 4.2.1
	s32 NumPermissionEntries; // Integer TODO new in 4.1.0
	s32 NumTypeEntries, NumComponentEntries, NumTaskEntries; // Integer TODO not in 1.2.10, not in 1.3.25
	s32 NumDirEntries, NumFileEntries, NumFileLocationEntries, NumIconEntries,
		NumIniEntries, NumRegistryEntries, NumInstallDeleteEntries,
		NumUninstallDeleteEntries, NumRunEntries, NumUninstallRunEntries; // Integer
	
	u32 LicenseSize, InfoBeforeSize, InfoAfterSize; // Cardinal TODO only in 1.2.10
	
	union {
		LegacySetupVersionData MinVersion, OnlyBelowVersion; // TODO only in 1.2.10
		SetupVersionData MinVersion, OnlyBelowVersion;
	};
	
	s32 BackColor; // LongInt
	s32 BackColor2; // LongInt TODO not in 1.2.10
	s32 WizardImageBackColor; // LongInt
	s32 WizardSmallImageBackColor; // LongInt TODO removed in 4.0.4, not in 1.2.10, not in 1.3.25
	union {
		s32 Password; // LongInt TODO removed in 4.2.0
		MD5Digest PasswordHash; // TODO only in [4.2.0, 5.3.9)
		SHA1Digest PasswordHash; // TODO new in 5.3.9
	};
	SetupSalt PasswordSalt; // TODO new in 4.2.2
	s32 ExtraDiskSpaceRequired; // LongInt TODO from 4.0.0: Integer64
	s32 SlicesPerDisk; // Integer TODO new in 4.0.0
	
	StoredEnum<InstallMode> installMode; // (imNormal, imSilent, imVerySilent) TODO removed in 5.0.0, not in 1.2.10, not in 1.3.25
	StoredEnum<UninstallLogMode> uninstallLogMode; // (lmAppend, lmNew, lmOverwrite) TODO not in 1.2.10
	StoredEnum<UninstallStyle> uninstallStyle; // (usClassic, usModern) TODO removed in 5.0.0, not in 1.2.10, not in 1.3.25
	StoredEnum<DirExistsWarning> dirExistsWarning; // (ddAuto, ddNo, ddYes) TODO not in 1.2.10
	StoredEnum<RestartComputer> restartComputer; // (rcAuto, rcNo, rcYes) TODO only in [3.0.0, 3.0.3)?
	StoredEnum<PrivilegesRequired> privilegesRequired; // (prNone, prPowerUser, prAdmin) TODO new in 3.0.4
	StoredEnum<ShowLanguageDialog> showLanguageDialog; // (slYes, slNo, slAuto) TODO new in 4.0.10
	StoredEnum<LanguageDetectionMethod> languageDetectionMethod; // (ldUILanguage, ldLocale, ldNone) TODO new in 4.0.10
	StoredEnum<CompressMethod> compressMethod; // CompressionMethod TODO new in 4.1.5
	StoredFlags<SetupProcessorArchitecture> architecturesAllowed, architecturesInstallIn64BitMode; // set of SetupProcessorArchitecture TODO new in 5.1.0
	
	s32 signedUninstallerOrigSize; // LongWord TODO only in [5.2.1, 5.3.10)
	u32 signedUninstallerHdrChecksum; // DWORD TODO only in [5.2.1, 5.3.10)
	
	StoredEnum<SetupDisablePage> disableDirPage, disableProgramGroupPage; // new in 5.3.3
	
	u32 UninstallDisplaySize; // Cardinal TODO new in 5.3.6
	
	StoredFlags<SetupHeaderOption> options; // set of SetupHeaderOption
	
};
*/

// ---------------------------------------------------------------------------------------

/* TODO remove:
 * - TSetupHeader
 * - TSetupVersionData
 * - TSetupHeaderOption
 * - TSetupProcessorArchitecture
 * - SetupHeaderStrings
 * - SetupID
 * - TSetupID
 */

/*

template <class Enum>
struct enum_size { };
#define ENUM_SIZE(Enum, Size) \
	template <> struct enum_size<Enum> { static const size_t value = (Size); }
#define ENUM_SIZE_LAST(Enum, Last) \
	ENUM_SIZE(Enum, (Last) + 1)
#define ENUM_SIZE_AUTO(Enum) \
	ENUM_SIZE(Enum, Enum ## __End)

template <class Enum>
class EnumSet {
	
public:
	
	typedef Enum enum_type;
	static const size_t count = enum_size<enum_type>::value;
	
private:
	
	typedef void(*unspecified_boolean_type)();
	typedef void(*zero_type)();
	
	typedef typename std::bitset<count> type;
	
	type value;
	
	EnumSet(type initial) : value(initial) { }
	
	
public:
	
	EnumSet() { }
	
	EnumSet(zero_type * zero) : value(0) { }
	
	EnumSet(enum_type flag) : value(0) { value.set(flag); }
	
	EnumSet(const EnumSet & other) : value(other.value) { }
	
	operator unspecified_boolean_type() const {
		return reinterpret_cast<unspecified_boolean_type>(value.any());
	}
	
	static EnumSet load(type value) {
		return EnumSet(value);
	}
	
	EnumSet & set() {
		value.set();
		return *this;
	}
	
	bool test(enum_type flag) const {
		return value.test(flag);
	}
	
	EnumSet operator&(EnumSet o) const { return EnumSet(value & o.value); }
	EnumSet operator|(EnumSet o) const { return EnumSet(value | o.value); }
	EnumSet operator~() const { return EnumSet(~value); }
	
	EnumSet & operator&=(EnumSet o) { value &= o.value; return *this; }
	EnumSet & operator|=(EnumSet o) { value |= o.value; return *this; }
	
	type bitset() {
		return value;
	}
	
};

template <class Enum>
static std::ostream & operator<<(std::ostream & os, EnumSet<Enum> rhs) {
	return os << rhs.bitset();
}





STORED_ENUM_MAP(UninstallLogModeMapper, lmUnknown, lmAppend, lmNew, lmOverwrite);
STORED_ENUM_MAP(DirExistsWarningMapper, ddUnknown, ddAuto, ddNo, ddYes);

template <size_t N>
struct StoredEnumType {
	
	static const size_t bits = boost::static_unsigned_max<8, next_power_of_two<log_next_power_of_two<N>::value >::value >::value;
	
	typedef typename boost::uint_t<bits>::exact type;
	
};
template <>
struct StoredEnumType<0> {
	
	static const size_t bits = 8;
	
	typedef typename boost::uint_t<bits>::exact type;
	
};

template <size_t N>
struct StoredFlagBits {
	
	static const size_t value = boost::static_unsigned_max<8, next_power_of_two<N>::value >::value;
	
};
template <size_t N, class Enable = void>
struct StoredFlagType {
	
	static const size_t bits = StoredFlagBits<N>::value;
	
	typedef typename boost::uint_t<bits>::exact type;
	
};
template <size_t N>
struct StoredFlagType<N, typename boost::enable_if_c<(StoredFlagBits<N>::value > 64 )>::type> {
	
	static const size_t bits = StoredFlagBits<N>::value;
	
	typedef typename std::bitset<bits> type;
	
};*/

/*
template <class EnumValueMap, class Type = typename StoredEnumType<EnumValueMap::count>::type>
struct StoredEnum {
	
	typedef Type stored_type;
	typedef typename EnumValueMap::enum_type enum_type;
	typedef EnumValueMap map;
	
	stored_type value;
	
	StoredEnum() { }
	StoredEnum(stored_type inital) : value(inital) { }
	
	enum_type get() const {
		
		for(size_t i = 0; i < map::count; i++) {
			if(value == i) {
				return map::values[i];
			}
		}
		
		return map::default_value;
	}
	
	operator enum_type() const {
		return get();
	}
	
};*/
/*
template <class EnumValueMap, class Type = typename StoredFlagType<EnumValueMap::count>::type>
struct StoredFlags {
	
	typedef Type stored_type;
	typedef typename EnumValueMap::enum_type enum_type;
	typedef EnumSet<enum_type> flag_type;
	typedef EnumValueMap map;
	
	stored_type value;
	
	StoredFlags() { }
	StoredFlags(stored_type inital) : value(inital) { }
	
	flag_type get() const {
		
		stored_type imask = 0;
		stored_type omask = 0;
		for(size_t i = 0; i < map::count; i++) {
			imask |= stored_type(map::values[i] == i) << i;
			omask |= stored_type(map::values[i] != i) << i;
		}
		
		flag_type result = flag_type::load(value & imask);
		
		if(omask) {
			
			stored_type tmp = value & omask;
			
			for(size_t i = 0; i < map::count; i++) {
				if(tmp & (stored_type(1) << i)) {
					result |= map::values[i];
				}
			}
		
		}
		
		return result;
	}
	
	operator flag_type() const {
		return get();
	}
	
	static stored_type get_imask() {
		
		stored_type imask = 0;
		for(size_t i = 0; i < map::count; i++) {
			imask |= stored_type(map::values[i] == i) << i;
		}
		
		return imask;
	}
	
	static stored_type get_omask() {
		
		stored_type omask = 0;
		for(size_t i = 0; i < map::count; i++) {
			omask |= stored_type(map::values[i] != i) << i;
		}
		
		return omask;
	}
	
};*/

//#pragma pack(pop)
