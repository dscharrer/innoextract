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
 * Structures for language entries stored in Inno Setup files.
 */
#ifndef INNOEXTRACT_SETUP_LANGUAGE_HPP
#define INNOEXTRACT_SETUP_LANGUAGE_HPP

#include <string>
#include <iosfwd>

#include <boost/cstdint.hpp>

#include "util/encoding.hpp"

namespace setup {

struct info;

struct language_entry {
	
	// introduced in 2.0.1
	
	std::string name;
	std::string language_name;
	std::string dialog_font;
	std::string title_font;
	std::string welcome_font;
	std::string copyright_font;
	std::string data;
	std::string license_text;
	std::string info_before;
	std::string info_after;
	
	boost::uint32_t language_id;
	boost::uint32_t codepage;
	size_t dialog_font_size;
	size_t dialog_font_standard_height;
	size_t title_font_size;
	size_t welcome_font_size;
	size_t copyright_font_size;
	
	bool right_to_left;
	
	void load(std::istream & is, const info & i);
	
	void decode(util::codepage_id cp);
	
};

} // namespace setup

#endif // INNOEXTRACT_SETUP_LANGUAGE_HPP
