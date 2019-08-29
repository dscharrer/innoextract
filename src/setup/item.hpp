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
 * Structures for setup items stored in Inno Setup files.
 */
#ifndef INNOEXTRACT_SETUP_ITEM_HPP
#define INNOEXTRACT_SETUP_ITEM_HPP

#include <string>
#include <iosfwd>

#include "setup/windows.hpp"

namespace setup {

struct info;
struct version;

struct item {
	
	std::string components;
	std::string tasks;
	std::string languages;
	std::string check;
	
	std::string after_install;
	std::string before_install;
	
	windows_version_range winver;
	
protected:
	
	void load_condition_data(std::istream & is, const info & i);
	
	void load_version_data(std::istream & is, const version & version) {
		winver.load(is, version);
	}
	
};

} // namespace setup

#endif // INNOEXTRACT_SETUP_ITEM_HPP
