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

#include <string>

#include <boost/filesystem/path.hpp>

#include "setup/filename.hpp"

struct extract_options {
	
	bool quiet;
	bool silent;
	
	bool warn_unused;
	
	bool list; // The --list action has been explicitely specified
	bool test; // The --test action has been explicitely specified
	bool extract; // The --extract action has been specified or automatically enabled
	bool gog_game_id; // The --gog-game-id action has been explicitely specified
	
	bool preserve_file_times;
	bool local_timestamps;
	
	std::string language;
	
	setup::filename_map filenames;
	
	boost::filesystem::path output_dir;
	
};

void process_file(const boost::filesystem::path & file, const extract_options & o);

#endif // INNOEXTRACT_CLI_EXTRACT_HPP
