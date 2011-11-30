
#ifndef INNOEXTRACT_SETUP_REGISTRY_HPP
#define INNOEXTRACT_SETUP_REGISTRY_HPP

#include <string>
#include <iosfwd>

#include "setup/item.hpp"
#include "setup/windows.hpp"
#include "util/enum.hpp"
#include "util/flags.hpp"

namespace setup {

struct version;

struct registry_entry : public item {
	
	FLAGS(flags,
		CreateValueIfDoesntExist,
		UninsDeleteValue,
		UninsClearValue,
		UninsDeleteEntireKey,
		UninsDeleteEntireKeyIfEmpty,
		PreserveStringType,
		DeleteKey,
		DeleteValue,
		NoError,
		DontCreateKey,
		Bits32,
		Bits64
	);
	
	enum hive_name {
		HKCR,
		HKCU,
		HKLM,
		HKU,
		HKPD,
		HKCC,
		HKDD,
		Unset,
	};
	
	enum value_type {
		None,
		String,
		ExpandString,
		DWord,
		Binary,
		MultiString,
		QWord,
	};
	
	std::string key;
	std::string name; // empty string means (Default) key
	std::string value;
	
	std::string permissions;
	
	hive_name hive;
	
	int permission; //!< index into the permission entry list
	
	value_type type;
	
	flags options;
	
	void load(std::istream & is, const version & version);
	
};

} // namespace setup

NAMED_FLAGS(setup::registry_entry::flags)
NAMED_ENUM(setup::registry_entry::hive_name)
NAMED_ENUM(setup::registry_entry::value_type)

#endif // INNOEXTRACT_SETUP_REGISTRY_HPP
