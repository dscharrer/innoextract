
#include "setup/RegistryEntry.hpp"

#include <stdint.h>

#include "util/load.hpp"
#include "util/storedenum.hpp"

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
	
	if(version < INNO_VERSION(1, 3, 21)) {
		::load<uint32_t>(is); // uncompressed size of the directory entry structure
	}
	
	is >> encoded_string(key, version.codepage());
	if(version.bits != 16) {
		is >> encoded_string(name, version.codepage());
	} else {
		name.clear();
	}
	is >> encoded_string(value, version.codepage());
	
	loadConditionData(is, version);
	
	if(version >= INNO_VERSION(4, 0, 11) && version < INNO_VERSION(4, 1, 0)) {
		is >> encoded_string(permissions, version.codepage());
	} else {
		permissions.clear();
	}
	
	loadVersionData(is, version);
	
	if(version.bits != 16) {
		hive = Hive(load_number<uint32_t>(is) & ~0x80000000);
	} else {
		hive = Unset;
	}
	
	if(version >= INNO_VERSION(4, 1, 0)) {
		permission = load_number<int16_t>(is);
	} else {
		permission = -1;
	}
	
	if(version >= INNO_VERSION(5, 2, 5)) {
		type = stored_enum<StoredRegistryEntryType2>(is).get();
	} else if(version.bits != 16) {
		type = stored_enum<StoredRegistryEntryType1>(is).get();
	} else {
		type = stored_enum<StoredRegistryEntryType0>(is).get();
	}
	
	stored_flag_reader<Options> flags(is);
	
	if(version.bits != 16) {
		flags.add(CreateValueIfDoesntExist);
		flags.add(UninsDeleteValue);
	}
	flags.add(UninsClearValue);
	flags.add(UninsDeleteEntireKey);
	flags.add(UninsDeleteEntireKeyIfEmpty);
	flags.add(PreserveStringType);
	if(version >= INNO_VERSION(1, 3, 21)) {
		flags.add(DeleteKey);
		flags.add(DeleteValue);
		flags.add(NoError);
		flags.add(DontCreateKey);
	}
	if(version >= INNO_VERSION(5, 1, 0)) {
		flags.add(Bits32);
		flags.add(Bits64);
	}
	
	options = flags;
}

ENUM_NAMES(RegistryEntry::Options, "Registry Option",
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
