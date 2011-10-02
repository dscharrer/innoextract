
#ifndef INNOEXTRACT_SETUP_REGISTRYENTRY_HPP
#define INNOEXTRACT_SETUP_REGISTRYENTRY_HPP

#include <iostream>

#include "setup/SetupCondition.hpp"
#include "setup/Version.hpp"
#include "setup/WindowsVersion.hpp"
#include "util/Enum.hpp"
#include "util/Flags.hpp"
#include "util/Types.hpp"

FLAGS(RegistryOptions,
	roCreateValueIfDoesntExist,
	roUninsDeleteValue,
	roUninsClearValue,
	roUninsDeleteEntireKey,
	roUninsDeleteEntireKeyIfEmpty,
	roPreserveStringType,
	roDeleteKey,
	roDeleteValue,
	roNoError,
	roDontCreateKey,
	ro32Bit,
	ro64Bit,
)

NAMED_ENUM(RegistryOptions::Enum)

struct RegistryEntry {
	
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
	
	SetupCondition condition;
	SetupTasks tasks;
	
	std::string permissions;
	
	WindowsVersion minVersion;
	WindowsVersion onlyBelowVersion;
	
	Hive hive;
	
	int permission; //!< index into the permission entry list
	
	Type type;
	
	RegistryOptions options;
	
	void load(std::istream & is, const InnoVersion & version);
	
};

NAMED_ENUM(RegistryEntry::Hive)

NAMED_ENUM(RegistryEntry::Type)

#endif // INNOEXTRACT_SETUP_REGISTRYENTRY_HPP
