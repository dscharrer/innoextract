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
 * Structures for setup tasks stored in Inno Setup files.
 */
#ifndef INNOEXTRACT_SETUP_TASK_HPP
#define INNOEXTRACT_SETUP_TASK_HPP

#include <string>
#include <iosfwd>

#include "setup/windows.hpp"
#include "util/enum.hpp"
#include "util/flags.hpp"

namespace setup {

struct info;

struct task_entry {
	
	// introduced in 2.0.0
	
	FLAGS(flags,
		Exclusive,
		Unchecked,
		Restart,
		CheckedOnce,
		DontInheritCheck
	);
	
	std::string name;
	std::string description;
	std::string group_description;
	std::string components;
	std::string languages;
	std::string check;
	
	int level;
	bool used;
	
	windows_version_range winver;
	
	flags options;
	
	void load(std::istream & is, const info & i);
	
};

} // namespace setup

NAMED_FLAGS(setup::task_entry::flags)

#endif // INNOEXTRACT_SETUP_TASK_HPP
