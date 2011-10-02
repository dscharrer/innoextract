
#include "setup/RegistryEntry.hpp"

#include "util/LoadingUtils.hpp"
#include "util/StoredEnum.hpp"

namespace {

// 16-bit
STORED_ENUM_MAP(StoredRegistryEntryType0, RegistryEntry::None,
	RegistryEntry::None,
	RegistryEntry::String,
);

STORED_ENUM_MAP(StoredRegistryEntryType1, RegistryEntry::None,
	RegistryEntry::None,
	RegistryEntry::String,
	RegistryEntry::ExpandString,
	RegistryEntry::DWord,
	RegistryEntry::Binary,
	RegistryEntry::MultiString,
);

// starting with version 5.2.5
STORED_ENUM_MAP(StoredRegistryEntryType2, RegistryEntry::None,
	RegistryEntry::None,
	RegistryEntry::String,
	RegistryEntry::ExpandString,
	RegistryEntry::DWord,
	RegistryEntry::Binary,
	RegistryEntry::MultiString,
	RegistryEntry::QWord,
);

} // anonymous namespace

void RegistryEntry::load(std::istream & is, const InnoVersion & version) {
	
	is >> EncodedString(key, version.codepage());
	if(version.bits != 16) {
		is >> EncodedString(name, version.codepage());
	} else {
		name.clear();
	}
	is >> EncodedString(value, version.codepage());
	condition.load(is, version);
	tasks.load(is, version);
	if(version >= INNO_VERSION(4, 0, 11) && version < INNO_VERSION(4, 1, 0)) {
		is >> EncodedString(permissions, version.codepage());
	} else {
		permissions.clear();
	}
	
	minVersion.load(is, version);
	onlyBelowVersion.load(is, version);
	
	if(version.bits != 16) {
		hive = Hive(loadNumber<u32>(is) & ~0x80000000);
	} else {
		hive = Unset;
	}
	
	if(version >= INNO_VERSION(4, 1, 0)) {
		permission = loadNumber<s16>(is);
	} else {
		permission = -1;
	}
	
	if(version >= INNO_VERSION(5, 2, 5)) {
		type = StoredEnum<StoredRegistryEntryType2>(is).get();
	} else if(version.bits != 16) {
		type = StoredEnum<StoredRegistryEntryType1>(is).get();
	} else {
		type = StoredEnum<StoredRegistryEntryType0>(is).get();
	}
	
	StoredFlagReader<RegistryOptions> flags(is);
	
	if(version.bits != 16) {
		flags.add(roCreateValueIfDoesntExist);
		flags.add(roUninsDeleteValue);
	}
	flags.add(roUninsClearValue);
	flags.add(roUninsDeleteEntireKey);
	flags.add(roUninsDeleteEntireKeyIfEmpty);
	flags.add(roPreserveStringType);
	if(version > INNO_VERSION(1, 2, 16)) {
		flags.add(roDeleteKey);
		flags.add(roDeleteValue);
		flags.add(roNoError);
		flags.add(roDontCreateKey);
	}
	if(version >= INNO_VERSION(5, 1, 0)) {
		flags.add(ro32Bit);
		flags.add(ro64Bit);
	}
	
	options = flags.get();	
}

ENUM_NAMES(RegistryOptions::Enum, "Registry Option",
	"create value if doesn't exist",
	"uninstall delete value",
	"uninstall clear value",
	"uninstall delete key",
	"uninstall delete key if empty",
	"preserve string type",
	"delete key",
	"delete value",
	"no error",
	"don't create key",
	"32 bit",
	"64 bit",
)

ENUM_NAMES(RegistryEntry::Hive, "Registry Hive",
	"HKCR",
	"HKCU",
	"HKLM",
	"HKU",
	"HKPD",
	"HKCC",
	"HKDD",
	"Unset",
)

ENUM_NAMES(RegistryEntry::Type, "Registry Entry Type",
	"none",
	"string",
	"expand string",
	"dword",
	"binary",
	"multi string",
	"qword",
)
