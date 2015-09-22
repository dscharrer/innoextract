/*
 * Copyright (C) 2011-2013 Daniel Scharrer
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

#include <cstring>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

#include <boost/foreach.hpp>
#include <boost/program_options.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>

#include "release.hpp"

#include "cli/extract.hpp"

#include "setup/version.hpp"

#include "util/console.hpp"
#include "util/log.hpp"
#include "util/time.hpp"
#include "util/windows.hpp"

namespace fs = boost::filesystem;
namespace po = boost::program_options;

enum ExitValues {
	ExitSuccess = 0,
	ExitUserError = 1,
	ExitDataError = 2
};

static const char * get_command(const char * argv0) {
	
	if(!argv0) {
		argv0 = innoextract_name;
	}
	std::string var = argv0;
	
#ifdef _WIN32
	size_t pos = var.find_last_of("/\\");
#else
	size_t pos = var.find_last_of('/');
#endif
	if(pos != std::string::npos) {
		var = var.substr(pos + 1);
	}
	
	var += "_COMMAND";
	
	const char * env = std::getenv(var.c_str());
	if(env) {
		return env;
	} else {
		return argv0;
	}
}

static void print_version(const extract_options & o) {
	if(o.silent) {
		std::cout << innoextract_version << '\n';
		return;
	}
	std::cout << color::white << innoextract_name
	          << ' ' << innoextract_version << color::reset
#ifdef DEBUG
	          << " (with debug output)"
#endif
	          << '\n';
	if(!o.quiet) {
		std::cout << "Extracts installers created by " << color::cyan
		          << innosetup_versions << color::reset << '\n';
	}
}

static void print_help(const char * name, const po::options_description & visible) {
	std::cout << color::white << "Usage: " << name << " [options] <setup file(s)>\n\n"
	          << color::reset;
	std::cout << "Extract files from an Inno Setup installer.\n";
	std::cout << "For multi-part installers only specify the exe file.\n";
	std::cout << visible << '\n';
	std::cout << "Extracts installers created by " << color::cyan
	          << innosetup_versions << color::reset << '\n';
	std::cout << '\n';
	std::cout << color::white << innoextract_name
	          << ' ' << innoextract_version << color::reset
	          << ' ' << innoextract_copyright << '\n';
	std::cout << "This is free software with absolutely no warranty.\n";
}

static void print_license() {
	
	std::cout << color::white << innoextract_name
	          << ' ' << innoextract_version << color::reset
	          << ' ' << innoextract_copyright << '\n';
	std::cout << '\n'<< innoextract_license << '\n';
	;
}

int main(int argc, char * argv[]) {
	
	po::options_description generic("Generic options");
	generic.add_options()
		("help,h", "Show supported options")
		("version,v", "Print version information")
		("license", "Show license information")
	;
	
	po::options_description action("Actions");
	action.add_options()
		("test,t", "Only verify checksums, don't write anything")
		("extract,e", "Extract files (default action)")
		("list,l", "Only list files, don't write anything")
		("gog-game-id", "Determine the GOG.com game ID for this installer")
	;
	
	po::options_description modifiers("Modifiers");
	modifiers.add_options()
		("dump", "Dump contents without converting filenames")
		("lowercase,L", "Convert extracted filenames to lower-case")
		("timestamps,T", po::value<std::string>(), "Timezone for file times or \"local\" or \"none\"")
		("output-dir,d", po::value<std::string>(), "Extract files into the given directory")
	;
	
	po::options_description filter("Filters");
	filter.add_options()
		("language", po::value<std::string>(), "Extract only files for this language")
		("include,I", po::value< std::vector<std::string> >(), "Extract only files that match this path")
	;
	
	po::options_description io("Display options");
	io.add_options()
		("quiet,q", "Output less information")
		("silent,s", "Output only error/warning information")
		("no-warn-unused", "Don't warn on unused .bin files")
		("color,c", po::value<bool>()->implicit_value(true), "Enable/disable color output")
		("progress,p", po::value<bool>()->implicit_value(true), "Enable/disable the progress bar")
		#ifdef DEBUG
			("debug,g", "Output debug information")
		#endif
	;
	
	po::options_description hidden("Hidden options");
	hidden.add_options()
		("setup-files", po::value< std::vector<std::string> >(), "Setup files to be extracted")
		/**/;
	
	po::options_description options_desc;
	options_desc.add(generic).add(action).add(modifiers).add(filter).add(io).add(hidden);
	
	po::options_description visible;
	visible.add(generic).add(action).add(modifiers).add(filter).add(io);
	
	po::positional_options_description p;
	p.add("setup-files", -1);
	
	po::variables_map options;
	
	// Parse the command-line.
	try {
		po::store(po::command_line_parser(argc, argv).options(options_desc).positional(p).run(),
		          options);
		po::notify(options);
	} catch(po::error & e) {
		color::init(color::disable, color::disable); // Be conservative
		std::cerr << "Error parsing command-line: " << e.what() << "\n\n";
		print_help(get_command(argv[0]), visible);
		return ExitUserError;
	}
	
	::extract_options o;
	
	// Verbosity settings.
	o.silent = (options.count("silent") != 0);
	o.quiet = o.silent || options.count("quiet");
	logger::quiet = o.quiet;
#ifdef DEBUG
	if(options.count("debug")) {
		logger::debug = true;
	}
#endif
	
	o.warn_unused = (options.count("no-warn-unused") == 0);
	
	// Color / progress bar settings.
	color::is_enabled color_e;
	po::variables_map::const_iterator color_i = options.find("color");
	if(color_i == options.end()) {
		color_e = o.silent ? color::disable : color::automatic;
	} else {
		color_e = color_i->second.as<bool>() ? color::enable : color::disable;
	}
	color::is_enabled progress_e;
	po::variables_map::const_iterator progress_i = options.find("progress");
	if(progress_i == options.end()) {
		progress_e = o.silent ? color::disable : color::automatic;
	} else {
		progress_e = progress_i->second.as<bool>() ? color::enable : color::disable;
	}
	color::init(color_e, progress_e);
	
	// Help output.
	if(options.count("help") != 0) {
		print_help(get_command(argv[0]), visible);
		return ExitSuccess;
	}
	
	// License output
	if(options.count("license") != 0) {
		print_license();
		return ExitSuccess;
	}
	
	// Main action.
	o.list = (options.count("list") != 0);
	o.extract = (options.count("extract") != 0);
	o.test = (options.count("test") != 0);
	o.gog_game_id = (options.count("gog-game-id") != 0);
	bool explicit_action = o.list || o.test || o.extract || o.gog_game_id;
	if(!explicit_action) {
		o.extract = true;
	}
	if(o.extract && o.test) {
		log_error << "Combining --extract and --test is not allowed!";
		return ExitUserError;
	}
	if(!o.extract && !o.test) {
		progress::set_enabled(false);
	}
	if(!o.silent && !o.gog_game_id) {
		o.list = true;
	}
	
	// Additional actions.
	o.filenames.set_expand(options.count("dump") == 0);
	o.filenames.set_lowercase(options.count("lowercase") != 0);
	
	// File timestamps
	{
		o.preserve_file_times = true, o.local_timestamps = false;
		po::variables_map::const_iterator i = options.find("timestamps");
		if(i != options.end()) {
			std::string timezone = i->second.as<std::string>();
			if(boost::iequals(timezone, "none")) {
				o.preserve_file_times = false;
			} else if(!boost::iequals(timezone, "UTC")) {
				o.local_timestamps = true;
				if(!boost::iequals(timezone, "local")) {
					util::set_local_timezone(timezone);
				}
			}
		}
	}
	
	// List version.
	if(options.count("version") != 0) {
		print_version(o);
		if(!explicit_action) {
			return ExitSuccess;
		}
	}
	
	{
		po::variables_map::const_iterator i = options.find("language");
		if(i != options.end()) {
			o.language = i->second.as<std::string>();
		}
	}

	{
		po::variables_map::const_iterator i = options.find("include");
		if(i != options.end()) {
			o.include = i->second.as<std::vector <std::string> >();
		}
	}
	
	if(options.count("setup-files") == 0) {
		if(!o.silent) {
			std::cout << get_command(argv[0]) << ": no input files specified\n";
			std::cout << "Try the --help (-h) option for usage information.\n";
		}
		return ExitSuccess;
	}
	
	{
		po::variables_map::const_iterator i = options.find("output-dir");
		if(i != options.end()) {
			/*
			 * We can't use fs::path directly with boost::program_options as fs::path's
			 * operator>> expects paths to be quoted if they contain spaces, breaking
			 * lexical casts.
			 * Instead, do the conversion in the assignment operator.
			 * See https://svn.boost.org/trac/boost/ticket/8535
			 */
			o.output_dir = i->second.as<std::string>();
			try {
				if(!o.output_dir.empty() && !fs::exists(o.output_dir)) {
					fs::create_directory(o.output_dir);
				}
			} catch(...) {
				log_error << "Could not create output directory " << o.output_dir;
				return ExitDataError;
			}
		}
	}
	
	const std::vector<std::string> & files = options["setup-files"]
	                                         .as< std::vector<std::string> >();
	
	bool suggest_bug_report = false;
	try {
		BOOST_FOREACH(const std::string & file, files) {
			process_file(file, o);
		}
	} catch(const std::ios_base::failure & e) {
		log_error << "Stream error while extracting files!\n"
		          << " └─ error reason: " << e.what();
		suggest_bug_report = true;
	} catch(const format_error & e) {
		log_error << e.what();
		suggest_bug_report = true;
	} catch(const std::runtime_error & e) {
		log_error << e.what();
	} catch(const setup::version_error &) {
		log_error << "Not a supported Inno Setup installer!";
	}
	
	if(suggest_bug_report) {
		std::cerr << color::blue << "If you are sure the setup file is not corrupted,"
		          << " consider \nfiling a bug report at "
		          << color::dim_cyan << innoextract_bugs << color::reset << '\n';
	}
	
	if(!logger::quiet || logger::total_errors || logger::total_warnings) {
		progress::clear();
		std::ostream & os = logger::quiet ? std::cerr : std::cout;
		os << color::green << "Done" << color::reset << std::dec;
		if(logger::total_errors || logger::total_warnings) {
			os << " with ";
			if(logger::total_errors) {
				os << color::red << logger::total_errors
				   << ((logger::total_errors == 1) ? " error" : " errors")
				   << color::reset;
			}
			if(logger::total_errors && logger::total_warnings) {
				os << " and ";
			}
			if(logger::total_warnings) {
				os << color::yellow << logger::total_warnings
				   << ((logger::total_warnings == 1) ? " warning" : " warnings")
				   << color::reset;
			}
		}
		os << '.' << std::endl;
	}
	
	return logger::total_errors == 0 ? ExitSuccess : ExitDataError;
}
