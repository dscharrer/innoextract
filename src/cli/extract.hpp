/*
 * Copyright (C) 2014-2020 Daniel Scharrer
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

#include <boost/cstdint.hpp>
#include <boost/filesystem/path.hpp>

#include "setup/filename.hpp"

struct format_error : public std::runtime_error {
	explicit format_error(const std::string & reason) : std::runtime_error(reason) { }
};

enum CollisionAction {
	OverwriteCollisions,
	RenameCollisions,
	RenameAllCollisions,
	ErrorOnCollisions
};

struct extract_options {
	
	bool quiet;
	bool silent;
	
	bool warn_unused; //!< Warn if there are unused files
	
	bool list_sizes; //!< Show size information for files
	bool list_checksums; //!< Show checksum information for files
	
	bool data_version; //!< Print the data version
	#ifdef DEBUG
	bool dump_headers; //!< Dump setup headers
	#endif
	bool list; //!< List files
	bool test; //!< Test files (but don't extract)
	bool extract; //!< Extract files
	bool list_languages; //!< List available languages
	bool gog_game_id; //!< Show the GOG.com game id
	bool show_password; //!< Show password check information
	bool check_password; //!< Abort if the provided password is incorrect
	
	bool preserve_file_times; //!< Set timestamps of extracted files
	bool local_timestamps; //!< Use local timezone for setting timestamps
	
	bool gog; //!< Try to extract additional archives used in GOG.com installers
	bool gog_galaxy; //!< Try to re-assemble GOG Galaxy files
	
	bool extract_unknown; //!< Try to extract unknown Inno Setup versions
	
	bool extract_temp; //!< Extract temporary files
	bool language_only; //!< Extract files not associated with any language
	std::string language; //!< Extract only files for this language
	std::vector<std::string> include; //!< Extract only files matching these patterns
	
	boost::uint32_t codepage;
	
	setup::filename_map filenames;
	CollisionAction collisions;
	std::string default_language;
	
	std::string password;
	
	boost::filesystem::path output_dir;
	
	extract_options()
		: quiet(false)
		, silent(false)
		, warn_unused(false)
		, list_sizes(false)
		, list_checksums(false)
		, data_version(false)
		, list(false)
		, test(false)
		, extract(false)
		, list_languages(false)
		, gog_game_id(false)
		, show_password(false)
		, check_password(false)
		, preserve_file_times(false)
		, local_timestamps(false)
		, gog(false)
		, gog_galaxy(false)
		, extract_unknown(false)
		, extract_temp(false)
		, language_only(false)
		, collisions(OverwriteCollisions)
	{ }
	
};

void process_file(const boost::filesystem::path & installer, const extract_options & o);

#endif // INNOEXTRACT_CLI_EXTRACT_HPP
