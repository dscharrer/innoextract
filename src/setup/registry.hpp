/*
 * Copyright (C) 2011-2019 Daniel Scharrer
 *
 * This software is provided 'as-is', without any express or implied
 * warranty.  In no event will the author(s) be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 */

/*!
 * \file
 *
 * Structures for Windows registry entries stored in Inno Setup files.
 */
#ifndef INNOEXTRACT_SETUP_REGISTRY_HPP
#define INNOEXTRACT_SETUP_REGISTRY_HPP

#include <string>
#include <iosfwd>

#include "setup/item.hpp"
#include "setup/windows.hpp"
#include "util/enum.hpp"
#include "util/flags.hpp"

namespace setup {

struct info;

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
	
	void load(std::istream & is, const info & i);
	
};

} // namespace setup

NAMED_FLAGS(setup::registry_entry::flags)
NAMED_ENUM(setup::registry_entry::hive_name)
NAMED_ENUM(setup::registry_entry::value_type)

#endif // INNOEXTRACT_SETUP_REGISTRY_HPP
