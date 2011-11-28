
#ifndef INNOEXTRACT_SETUP_REGISTRYENTRY_HPP
#define INNOEXTRACT_SETUP_REGISTRYENTRY_HPP

#include <string>
#include <iosfwd>

#include "setup/SetupItem.hpp"
#include "setup/version.hpp"
#include "setup/WindowsVersion.hpp"
#include "util/enum.hpp"
#include "util/flags.hpp"

struct RegistryEntry : public SetupItem {
	
	FLAGS(Options,
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
	
	enum Hive {
		HKCR,
		HKCU,
		HKLM,
		HKU,
		HKPD,
		HKCC,
		HKDD,
		Unset,
	};
	
	enum Type {
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
	
	Hive hive;
	
	int permission; //!< index into the permission entry list
	
	Type type;
	
	Options options;
	
	void load(std::istream & is, const inno_version & version);
	
};

FLAGS_OVERLOADS(RegistryEntry::Options)
NAMED_ENUM(RegistryEntry::Options)

NAMED_ENUM(RegistryEntry::Hive)

NAMED_ENUM(RegistryEntry::Type)

#endif // INNOEXTRACT_SETUP_REGISTRYENTRY_HPP
