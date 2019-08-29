/*
 * Copyright (C) 2012-2019 Daniel Scharrer
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
 * Map for converting between stored filenames and output filenames.
 */
#ifndef INNOEXTRACT_SETUP_FILENAME_HPP
#define INNOEXTRACT_SETUP_FILENAME_HPP

#include <string>
#include <map>

namespace setup {

//! Separator to use for output paths.
#if defined(_WIN32)
static const char path_sep = '\\';
#else
static const char path_sep = '/';
#endif

/*!
 * Map to convert between raw windows file paths stored in the setup file (which can
 * contain variables) and output filenames.
 */
class filename_map : public std::map<std::string, std::string> {
	
	std::string lookup(const std::string & key) const;
	
	bool lowercase;
	bool expand;
	
	typedef std::string::const_iterator it;
	
	std::string expand_variables(it & begin, it end, bool close = false) const;
	static std::string shorten_path(const std::string & path);
	
public:
	
	filename_map() : lowercase(false), expand(false) { }
	
	std::string convert(std::string path) const;
	
	//! Set if paths should be converted to lower-case.
	void set_lowercase(bool enable) { lowercase = enable; }
	
	//! Set if paths should be converted to lower-case.
	bool is_lowercase() const { return lowercase; }
	
	//! Set if variables should be expanded and path separators converted.
	void set_expand(bool enable) { expand = enable; }
	
};

} // namespace setup

#endif // INNOEXTRACT_SETUP_FILENAME_HPP
