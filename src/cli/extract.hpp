/*
 * Copyright (C) 2014 Daniel Scharrer
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
 * Routines to extract/list files from an Inno Setup archive.
 */
#ifndef INNOEXTRACT_CLI_EXTRACT_HPP
#define INNOEXTRACT_CLI_EXTRACT_HPP

#include <stdexcept>
#include <string>
#include <vector>

#include <boost/filesystem/path.hpp>

#include "setup/filename.hpp"

struct format_error : public std::runtime_error {
	explicit format_error(const std::string & reason) : std::runtime_error(reason) { }
};

struct extract_options {
	
	bool quiet;
	bool silent;
	
	bool warn_unused; //!< Warn if there are unused files
	
	bool list; //!< List files
	bool test; //!< Test files (but don't extract)
	bool extract; //!< Extract files
	bool list_languages; //!< List available languages
	bool gog_game_id; //!< Show the GOG.com game id
	
	bool preserve_file_times; //!< Set timestamps of extracted files
	bool local_timestamps; //!< Use local timezone for setting timestamps
	
	std::string language; //!< Extract only files for this language
	std::vector<std::string> include; //!< Extract only files matching these patterns
	
	setup::filename_map filenames;
	
	boost::filesystem::path output_dir;
	
};

void process_file(const boost::filesystem::path & file, const extract_options & o);

#endif // INNOEXTRACT_CLI_EXTRACT_HPP
