/*
 * Copyright (C) 2011-2020 Daniel Scharrer
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
#include "util/fstream.hpp"
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
		("list-sizes", "List file sizes")
		("list-checksums", "List file checksums")
		("info,i", "Print information about the installer")
		("list-languages", "List languages supported by the installer")
		("gog-game-id", "Determine the installer's GOG.com game ID")
		("show-password", "Show password check information")
		("check-password", "Abort if the password is incorrect")
		("data-version,V", "Only print the data version")
		#ifdef DEBUG
		("dump-headers", "Dump decompressed setup headers")
		#endif
	;
	
	po::options_description modifiers("Modifiers");
	modifiers.add_options()
		("codepage", po::value<boost::uint32_t>(), "Encoding for ANSI strings")
		("collisions", po::value<std::string>(), "How to handle duplicate files")
		("default-language", po::value<std::string>(), "Default language for renaming")
		("dump", "Dump contents without converting filenames")
		("lowercase,L", "Convert extracted filenames to lower-case")
		("timestamps,T", po::value<std::string>(), "Timezone for file times or \"local\" or \"none\"")
		("output-dir,d", po::value<std::string>(), "Extract files into the given directory")
		("password,P", po::value<std::string>(), "Password for encrypted files")
		("password-file", po::value<std::string>(), "File to load password from")
		("gog,g", "Extract additional archives from GOG.com installers")
		("no-gog-galaxy", "Don't re-assemble GOG Galaxy file parts")
		("no-extract-unknown,n", "Don't extract unknown Inno Setup versions")
	;
	
	po::options_description filter("Filters");
	filter.add_options()
		("exclude-temp,m", "Don't extract temporary files")
		("language", po::value<std::string>(), "Extract only files for this language")
		("language-only", "Only extract language-specific files")
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
		("debug", "Output debug information")
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
	} catch(std::exception & e) {
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
	o.list_sizes = (options.count("list-sizes") != 0);
	o.list_checksums = (options.count("list-checksums") != 0);
	bool explicit_list = (options.count("list") != 0);
	o.list = explicit_list || o.list_sizes || o.list_checksums;
	o.extract = (options.count("extract") != 0);
	o.test = (options.count("test") != 0);
	o.list_languages = (options.count("list-languages") != 0);
	o.gog_game_id = (options.count("gog-game-id") != 0);
	o.show_password = (options.count("show-password") != 0);
	o.check_password = (options.count("check-password") != 0);
	if(options.count("info") != 0) {
		o.list_languages = true;
		o.gog_game_id = true;
		o.show_password = true;
	}
	bool explicit_action = o.list || o.test || o.extract || o.list_languages
	                       || o.gog_game_id || o.show_password || o.check_password;
	if(!explicit_action) {
		o.extract = true;
	}
	if(!o.extract && !o.test) {
		progress::set_enabled(false);
	}
	if(!o.silent && (o.test || o.extract)) {
		o.list = true;
	}
	if(!o.quiet && explicit_list) {
		o.list_sizes = true;
	}
	
	// Additional actions.
	o.filenames.set_expand(options.count("dump") == 0);
	o.filenames.set_lowercase(options.count("lowercase") != 0);
	
	// File timestamps
	{
		o.preserve_file_times = true, o.local_timestamps = false;
		po::variables_map::const_iterator i = options.find("timestamps");
		if(i != options.end()) {
			std::string timezone_name = i->second.as<std::string>();
			if(boost::iequals(timezone_name, "none")) {
				o.preserve_file_times = false;
			} else if(!boost::iequals(timezone_name, "UTC")) {
				o.local_timestamps = true;
				if(!boost::iequals(timezone_name, "local")) {
					util::set_local_timezone(timezone_name);
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
		po::variables_map::const_iterator i = options.find("codepage");
		o.codepage = (i != options.end()) ? i->second.as<boost::uint32_t>() : 0;
	}
	{
		o.collisions = OverwriteCollisions;
		po::variables_map::const_iterator i = options.find("collisions");
		if(i != options.end()) {
			std::string collisions = i->second.as<std::string>();
			if(collisions == "overwrite")  {
				o.collisions = OverwriteCollisions;
			} else if(collisions == "rename") {
				o.collisions = RenameCollisions;
			} else if(collisions == "rename-all") {
				o.collisions = RenameAllCollisions;
			} else if(collisions == "error") {
				o.collisions = ErrorOnCollisions;
			} else {
				log_error << "Unsupported --collisions value: " << collisions;
				return ExitUserError;
			}
		}
	}
	{
		po::variables_map::const_iterator i = options.find("default-language");
		if(i != options.end()) {
			o.default_language = i->second.as<std::string>();
		}
	}
	
	o.extract_temp = (options.count("exclude-temp") == 0);
	{
		po::variables_map::const_iterator i = options.find("language");
		if(i != options.end()) {
			o.language = i->second.as<std::string>();
		}
		o.language_only = (options.count("language-only") != 0);
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
		}
	}
	
	{
		po::variables_map::const_iterator password = options.find("password");
		po::variables_map::const_iterator password_file = options.find("password-file");
		if(password != options.end() && password_file != options.end()) {
			log_error << "Combining --password and --password-file is not allowed";
			return ExitUserError;
		}
		if(password != options.end()) {
			o.password = password->second.as<std::string>();
		}
		if(password_file != options.end()) {
			std::istream * is = &std::cin;
			fs::path file = password_file->second.as<std::string>();
			util::ifstream ifs;
			if(file != "-") {
				ifs.open(file);
				if(!ifs.is_open()) {
					log_error << "Could not open password file " << file;
					return ExitDataError;
				}
				is = &ifs;
			}
			std::getline(*is, o.password);
			if(!o.password.empty() && o.password[o.password.size() - 1] == '\n') {
				o.password.resize(o.password.size() - 1);
			}
			if(!o.password.empty() && o.password[o.password.size() - 1] == '\r') {
				o.password.resize(o.password.size() - 1);
			}
			if(!*is) {
				log_error << "Could not read password file " << file;
				return ExitDataError;
			}
		}
		if(o.check_password && o.password.empty()) {
			log_error << "Combining --check-password requires a password";
			return ExitUserError;
		}
	}
	
	o.gog = (options.count("gog") != 0);
	o.gog_galaxy = (options.count("no-gog-galaxy") == 0);
	
	o.data_version = (options.count("data-version") != 0);
	if(o.data_version) {
		logger::quiet = true;
		if(explicit_action) {
			log_error << "Combining --data-version with other options is not allowed";
			return ExitUserError;
		}
	}
	
	#ifdef DEBUG
	o.dump_headers = (options.count("dump-headers") != 0);
	if(o.dump_headers) {
		if(explicit_action || o.data_version) {
			log_error << "Combining --dump-headers with other options is not allowed";
			return ExitUserError;
		}
	}
	#endif
	
	o.extract_unknown = (options.count("no-extract-unknown") == 0);
	
	const std::vector<std::string> & files = options["setup-files"]
	                                         .as< std::vector<std::string> >();
	
	bool suggest_bug_report = false;
	try {
		BOOST_FOREACH(const std::string & file, files) {
			process_file(file, o);
			if(!o.data_version && files.size() > 1) {
				std::cout << '\n';
			}
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
