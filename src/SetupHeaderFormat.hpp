
#include "Types.h"

#include <string>
#include <bitset>
#include <iostream>

#include <boost/utility/enable_if.hpp>
#include <boost/integer.hpp>
#include <boost/integer/static_log2.hpp>
#include <boost/integer/static_min_max.hpp>
#include <boost/integer_traits.hpp>

#pragma pack(push,1)

/*
//   2.0.8, 2.0.11
enum SetupHeaderOption_20008 {
	shDisableStartupPrompt,
	shUninstallable,
	shCreateAppDir,
	shDisableDirPage,
	shDisableProgramGroupPage,
	shAllowNoIcons,
	shAlwaysRestart,
	shAlwaysUsePersonalGroup,
	shWindowVisible,
	shWindowShowCaption,
	shWindowResizable,
	shWindowStartMaximized,
	shEnableDirDoesntExistWarning,
	shDisableAppendDir,
	shPassword,
	shAllowRootDirectory,
	shDisableFinishedPage,
	shAdminPrivilegesRequired,
	shAlwaysCreateUninstallIcon,
	shChangesAssociations,
	shCreateUninstallRegKey,
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
};

//   5.2.3
enum SetupHeaderOption_50203 {
	shDisableStartupPrompt,
	shUninstallable,
	shCreateAppDir,
	shDisableDirPage,
	shDisableProgramGroupPage,
	shAllowNoIcons,
	shAlwaysRestart,
	shAlwaysUsePersonalGroup,
	shWindowVisible,
	shWindowShowCaption,
	shWindowResizable,
	shWindowStartMaximized,
	shEnableDirDoesntExistWarning,
	// -shDisableAppendDir
	shPassword,
	shAllowRootDirectory,
	shDisableFinishedPage,
	// -shAdminPrivilegesRequired
	// -shAlwaysCreateUninstallIcon
	shChangesAssociations,
	shCreateUninstallRegKey,
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
	// New:
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
};

typedef u8 MD5Digest[16];
typedef u8 SetupSalt[8];

//   2.0.8, 2.0.11
struct SetupVersionData {
	s32 WinVersion, NTVersion; // Cardinal
	s16 NTServicePack; // Word
};

//   2.0.8, 2.0.11
struct SetupHeader_20008 {
	
	const size_t numstrings;
	
	std::string AppName, AppVerName, AppId, AppCopyright, AppPublisher, AppPublisherURL,
		AppSupportURL, AppUpdatesURL, AppVersion, DefaultDirName,
		DefaultGroupName, UninstallIconName, BaseFilename, LicenseText,
		InfoBeforeText, InfoAfterText, UninstallFilesDir, UninstallDisplayName,
		UninstallDisplayIcon, AppMutex; // String
	
	CharSet LeadBytes;
	
	s32 NumTypeEntries, NumComponentEntries, NumTaskEntries; // Integer
	s32 NumDirEntries, NumFileEntries, NumFileLocationEntries, NumIconEntries,
		NumIniEntries, NumRegistryEntries, NumInstallDeleteEntries,
		NumUninstallDeleteEntries, NumRunEntries, NumUninstallRunEntries; // Integer
	SetupVersionData MinVersion, OnlyBelowVersion;
	s32 BackColor, BackColor2, WizardImageBackColor; // LongInt
	s32 WizardSmallImageBackColor; // LongInt
	s32 Password; // LongInt
	s32 ExtraDiskSpaceRequired; // LongInt
	u8 InstallMode; // (imNormal, imSilent, imVerySilent);
	u8 UninstallLogMode; // (lmAppend, lmNew, lmOverwrite);
	u8 UninstallStyle; // (usClassic, usModern);
	u8 DirExistsWarning; // (ddAuto, ddNo, ddYes);
	u64 Options; // set of SetupHeaderOption_20008
	
}; */

/*
#define ARRAY_SIZE(array) (sizeof(array)/sizeof(*(array)))


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

template <class Enum>
struct EnumValueMap {
	
	typedef Enum enum_type;
	typedef Enum flag_type;
	
};

template <class Enum>
struct EnumValueMap<EnumSet<Enum> > {
	
	typedef Enum enum_type;
	typedef EnumSet<Enum> flag_type;
	
};*/

namespace boost {
	template <class T>
	class integer_traits<const T> : public integer_traits<T> { };
}

template <size_t N, class Type = void, class Enable = void>
struct is_power_of_two {
	static const bool value = false;
};
template <size_t N, class Type>
struct is_power_of_two<N, Type, typename boost::enable_if_c<(N & (N - 1)) == 0>::type> {
	static const bool value = true;
	typedef Type type;
};

template <size_t N, class Enable = void>
struct log_next_power_of_two {
	static const size_t value = boost::static_log2<N>::value + 1;
};
template <size_t N>
struct log_next_power_of_two<N, typename boost::enable_if<is_power_of_two<N> >::type> {
	static const size_t value = boost::static_log2<N>::value;
};

template <size_t N, class Enable = void>
struct next_power_of_two {
	static const size_t value = size_t(1) << (boost::static_log2<N>::value + 1);
};
template <size_t N>
struct next_power_of_two<N, typename boost::enable_if<is_power_of_two<N> >::type> {
	static const size_t value = N;
};

template <size_t Bits> struct fast_type_impl { };

template <> struct fast_type_impl<8> { typedef uint_fast8_t type; };
template <> struct fast_type_impl<16> { typedef uint_fast16_t type; };
template <> struct fast_type_impl<32> { typedef uint_fast32_t type; };
template <> struct fast_type_impl<64> { typedef uint_fast64_t type; };
template <> struct fast_type_impl<128> { typedef __uint128_t type; };

template <size_t Bits>
struct fast_type : public fast_type_impl< boost::static_unsigned_max<8, next_power_of_two<Bits>::value>::value > { };

struct BitsetConverter {
	
private:
	
	typedef ptrdiff_t shift_type;
	typedef size_t index_type;
	
	template <class Combiner, class Entry>
	struct IterateEntries {
		static const typename Combiner::type value = Combiner::template combine<Entry, (IterateEntries<Combiner, typename Entry::next>::value)>::value;
	};
	template <class Combiner> struct IterateEntries<Combiner, void> { static const typename Combiner::type value = Combiner::base; };
	template <class Type, Type Base> struct Combiner { typedef Type type; static const Type base = Base; };
	
	template<class Getter, class Type>
	struct MaxCombiner : public Combiner<Type, boost::integer_traits<Type>::const_min> {
		template <class Entry, Type accumulator>
		struct combine { static const Type value = boost::static_signed_max<Getter::template get<Entry>::value, accumulator>::value; };
	};
	
	template<class Getter, class Type>
	struct MinCombiner : public Combiner<Type, boost::integer_traits<Type>::const_max> {
		template <class Entry, Type accumulator>
		struct combine { static const Type value = boost::static_signed_min<Getter::template get<Entry>::value, accumulator>::value; };
	};
	
	struct ShiftGetter { template<class Entry> struct get { static const shift_type value = Entry::shift; }; };
	struct FromGetter { template<class Entry> struct get { static const index_type value = Entry::from; }; };
	struct ToGetter { template<class Entry> struct get { static const index_type value = Entry::to; }; };
	
	template<shift_type Shift, class Type>
	struct ShiftMaskCombiner : public Combiner<Type, Type(0)> {
		template <class Entry, Type mask>
		struct combine { static const Type value = mask | ( (Entry::shift == Shift) ? (Type(1) << Entry::from) : Type(0) ); };
	};
	
	template<class List>
	struct Builder;
	
	template<index_type From, index_type To, class Next = void>
	struct Entry {
		
		typedef Entry<From, To, Next> This;
		
		static const index_type from = From;
		static const index_type to = To;
		typedef Next next;
		
		static const shift_type shift = shift_type(from) - shift_type(to);
		
		static const shift_type max_shift = IterateEntries<MaxCombiner<ShiftGetter, shift_type>, This>::value;
		static const shift_type min_shift = IterateEntries<MinCombiner<ShiftGetter, shift_type>, This>::value;
		
		static const index_type max_from = IterateEntries<MaxCombiner<FromGetter, index_type>, This>::value;
		typedef typename fast_type<max_from + 1>::type in_type;
		
		static const index_type max_to = IterateEntries<MaxCombiner<ToGetter, index_type>, This>::value;
		typedef typename fast_type<max_to + 1>::type out_type;
		
		template<shift_type Shift> struct ShiftMask { static const in_type value = IterateEntries<ShiftMaskCombiner<Shift, in_type>, This>::value; };
		
		template <shift_type Shift> inline static typename boost::enable_if_c<(Shift >= shift_type(0)), out_type>::type evaluate(in_type value) {
			return out_type((value & ShiftMask<Shift>::value) >> Shift);
		}
		template <shift_type Shift> inline static typename boost::enable_if_c<(Shift < shift_type(0)), out_type>::type evaluate(in_type value) {
			return out_type(value & ShiftMask<Shift>::value) << (-Shift); 
		}
		
		template<shift_type Shift, class Enable = void> struct NextShift { static const shift_type value = Shift + 1; };
		template<shift_type Shift>
		struct NextShift<Shift, typename boost::enable_if_c<Shift != max_shift && ShiftMask<Shift + 1>::value == in_type(0)>::type > {
			static const shift_type value = NextShift<Shift + 1>::value;
		};
		
		template <shift_type Shift>
		inline static typename boost::enable_if_c<(NextShift<Shift>::value != max_shift + 1), out_type>::type map(in_type value) {
			return evaluate<Shift>(value) | (map<NextShift<Shift>::value>(value));
		}
		template <shift_type Shift>
		inline static typename boost::enable_if_c<(NextShift<Shift>::value == max_shift + 1), out_type>::type map(in_type value) {
			return evaluate<Shift>(value);
		}
		
	public:
		
		typedef Builder<This> add;
		
		static out_type convert(in_type value) {
			return map<min_shift>(value);
		}
		
	};
	
	template<class List>
	struct Builder {
		
		template<index_type From, index_type To>
		struct map : public Entry<From, To, List> { };
		
		template<index_type To, class Current = List>
		struct value : public Entry<Current::from + 1, To, Current> { };
		
		template<index_type To>
		struct value<To, void> : public Entry<0, To> { };
		
	};
	
	
public:
	
	typedef Builder<void> add;
	
};






/*




#define STORED_ENUM_MAP(MapName, Default, ...) \
struct MapName : public EnumValueMap<typeof(Default)> { \
	static const flag_type default_value; \
	static const flag_type values[]; \
	static const size_t count; \
}; \
const MapName::flag_type MapName::default_value = Default; \
const MapName::flag_type MapName::values[] = { __VA_ARGS__ }; \
const size_t MapName::count = ARRAY_SIZE(MapName::values)

template <class Enum>
struct EnumNames {
	
	const size_t count;
	
	const char * name;
	
	const char * names[0];
	
};

#define ENUM_NAMES(Enum, Default, ...)

enum UninstallLogMode { lmAppend, lmNew, lmOverwrite, lmUnknown };
ENUM_NAMES(UninstallLogMode, "Append", "New", "Overwrite");

enum DirExistsWarning { ddAuto, ddNo, ddYes, ddUnknown };
ENUM_NAMES(DirExistsWarning, "Auto", "No", "Yes");

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

/*
//   5.2.3
struct SetupHeader_50203 {
	
	std::string AppName, AppVerName, AppId, AppCopyright, AppPublisher, AppPublisherURL,
		AppSupportPhone, AppSupportURL, AppUpdatesURL, AppVersion, DefaultDirName,
		DefaultGroupName, BaseFilename, LicenseText,
		InfoBeforeText, InfoAfterText, UninstallFilesDir, UninstallDisplayName,
		UninstallDisplayIcon, AppMutex, DefaultUserInfoName,
		DefaultUserInfoOrg, DefaultUserInfoSerial, CompiledCodeText,
		AppReadmeFile, AppContact, AppComments, AppModifyPath,
		SignedUninstallerSignature;
		
	CharSet LeadBytes;
	
	s32 NumLanguageEntries, NumCustomMessageEntries, NumPermissionEntries,
		NumTypeEntries, NumComponentEntries, NumTaskEntries, NumDirEntries,
		NumFileEntries, NumFileLocationEntries, NumIconEntries, NumIniEntries,
		NumRegistryEntries, NumInstallDeleteEntries, NumUninstallDeleteEntries,
		NumRunEntries, NumUninstallRunEntries; // Integer
	SetupVersionData MinVersion, OnlyBelowVersion;
	s32 BackColor, BackColor2, WizardImageBackColor; // LongInt
	MD5Digest PasswordHash;
	u64 PasswordSalt; // array[0..7] of Byte
	s64 ExtraDiskSpaceRequired; // Integer64
	s32 SlicesPerDisk; // Integer
	StoredEnum<UninstallLogModeMapper> uninstallLogMode; // (lmAppend, lmNew, lmOverwrite)
	u8 DirExistsWarning; // (ddAuto, ddNo, ddYes)
	u8 PrivilegesRequired; // (prNone, prPowerUser, prAdmin)
	u8 ShowLanguageDialog; // (slYes, slNo, slAuto)
	LanguageDetectionMethod: (ldUILanguage, ldLocale, ldNone);
	CompressMethod: TSetupCompressMethod;
	ArchitecturesAllowed, ArchitecturesInstallIn64BitMode: TSetupProcessorArchitectures;
	SignedUninstallerOrigSize: LongWord;
	SignedUninstallerHdrChecksum: DWORD;
	u64 Options; // set of SetupHeaderOption_50203;
}; */

#pragma pack(pop)
