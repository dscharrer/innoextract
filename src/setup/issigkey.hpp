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
 * Structures for ISSig public key entries stored in Inno Setup 6.5.0+ files.
 */
#ifndef INNOEXTRACT_SETUP_ISSIGKEY_HPP
#define INNOEXTRACT_SETUP_ISSIGKEY_HPP

#include <string>
#include <iosfwd>

namespace setup {

struct info;

struct issig_key_entry {
	
	// introduced in 6.5.0
	
	// X coordinate of the Ed25519 public key, as raw bytes.
	std::string public_x;
	
	// Y coordinate of the Ed25519 public key, as raw bytes.
	std::string public_y;
	
	// Tool-supplied identifier (typically the ISSigTool key file name).
	std::string runtime_id;
	
	void load(std::istream & is, const info & i);
	
};

} // namespace setup

#endif // INNOEXTRACT_SETUP_ISSIGKEY_HPP
