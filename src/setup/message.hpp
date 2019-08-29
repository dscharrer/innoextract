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
 * Structures for custom localized messages stored in Inno Setup files.
 */
#ifndef INNOEXTRACT_SETUP_MESSAGE_HPP
#define INNOEXTRACT_SETUP_MESSAGE_HPP

#include <string>
#include <iosfwd>

namespace setup {

struct info;

struct message_entry {
	
	// introduced in 4.2.1
	
	// UTF-8 encoded name.
	std::string name;
	
	// Value encoded in the codepage specified at language index.
	std::string value;
	
	// Index into the default language entry list or -1.
	int language;
	
	void load(std::istream & is, const info & i);
	
};

} // namespace setup

#endif // INNOEXTRACT_SETUP_MESSAGE_HPP
