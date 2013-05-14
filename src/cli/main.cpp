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

#include <algorithm>
#include <cctype>
#include <cstring>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <map>
#include <string>
#include <vector>

#include <boost/algorithm/string/predicate.hpp>
#include <boost/foreach.hpp>
#include <boost/make_shared.hpp>
#include <boost/program_options.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>

#include "release.hpp"

#include "cli/debug.hpp"

#include "loader/offsets.hpp"

#include "setup/data.hpp"
#include "setup/expression.hpp"
#include "setup/file.hpp"
#include "setup/filename.hpp"
#include "setup/info.hpp"
#include "setup/version.hpp"

#include "stream/chunk.hpp"
#include "stream/file.hpp"
#include "stream/slice.hpp"

#include "util/boostfs_compat.hpp"
#include "util/console.hpp"
#include "util/fstream.hpp"
#include "util/load.hpp"
#include "util/log.hpp"
#include "util/output.hpp"
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


struct options {
	
	bool quiet;
	bool silent;
	
	bool list; // The --list action has been explicitely specified
	bool test; // The --test action has been explicit specified
	bool extract; // The --extract action has been specified or automatically enabled
	
	bool preserve_file_times;
	bool local_timestamps;
	
	std::string language;
	
	setup::filename_map filenames;
	
	fs::path output_dir;
	
};


static void print_version(const options & o) {
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


struct file_output {
	
	fs::path name;
	util::ofstream stream;
	
	explicit file_output(const fs::path & file) : name(file) {
		try {
			fs::create_directories(name.parent_path());
		} catch(...) {
			throw std::runtime_error("error creating directories for \""
			                         + name.string() + '"');
		}
		stream.open(name, std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
		if(!stream.is_open()) {
			throw std::runtime_error("error opening output file \"" + name.string() + '"');
		}
	}
	
};


static void process_file(const fs::path & file, const options & o) {
	
	bool is_directory;
	try {
		is_directory = fs::is_directory(file);
	} catch(...) {
		throw std::runtime_error("error opening file \"" + file.string()
		                         + "\": access denied");
	}
	if(is_directory) {
		throw std::runtime_error("input file \"" + file.string() + "\" is a directory");
	}
	
	util::ifstream ifs(file, std::ios_base::in | std::ios_base::binary);
	if(!ifs.is_open()) {
		throw std::runtime_error("error opening file \"" + file.string() + '"');
	}
	
	loader::offsets offsets;
	offsets.load(ifs);
	
	std::cout << std::boolalpha;
	
#ifdef DEBUG
	if(logger::debug) {
		print_offsets(offsets);
		std::cout << '\n';
	}
#endif
	
	setup::info::entry_types entries = setup::info::DataEntries | setup::info::Files;
#ifdef DEBUG
	if(logger::debug) {
		entries = setup::info::entry_types::all();
	}
#endif
	
	ifs.seekg(offsets.header_offset);
	setup::info info;
	info.load(ifs, entries);
	
	if(!o.quiet) {
		const std::string & name = info.header.app_versioned_name.empty()
		                           ? info.header.app_name : info.header.app_versioned_name;
		std::cout << (o.extract ? "Extracting" : o.test ? "Testing" : "Listing")
		          << " \"" << color::green << name << color::reset
		          << "\" - setup data version " << color::white << info.version << color::reset
		          << std::endl;
	}
	
#ifdef DEBUG
	if(logger::debug) {
		std::cout << '\n';
		print_info(info);
		std::cout << '\n';
	}
#endif
	
	boost::uint64_t total_size = 0;
	
	std::vector< std::vector<size_t> > files_for_location;
	files_for_location.resize(info.data_entries.size());
	for(size_t i = 0; i < info.files.size(); i++) {
		if(info.files[i].location < files_for_location.size()) {
			files_for_location[info.files[i].location].push_back(i);
		}
	}
	
	typedef std::map<stream::file, size_t> Files;
	typedef std::map<stream::chunk, Files> Chunks;
	Chunks chunks;
	for(size_t i = 0; i < info.data_entries.size(); i++) {
		setup::data_entry & location = info.data_entries[i];
		if(location.chunk.compression == stream::UnknownCompression) {
			location.chunk.compression = info.header.compression;
		}
		chunks[location.chunk][location.file] = i;
		total_size += location.file.size;
	}
	
	boost::scoped_ptr<stream::slice_reader> slice_reader;
	if(o.extract || o.test) {
		if(offsets.data_offset) {
			slice_reader.reset(new stream::slice_reader(&ifs, offsets.data_offset));
		} else {
			slice_reader.reset(new stream::slice_reader(file.parent_path(), file.stem(),
			                                            info.header.slices_per_disk));
		}
	}
	
	progress extract_progress(total_size);
	
	BOOST_FOREACH(const Chunks::value_type & chunk, chunks) {
		
		debug("[starting " << chunk.first.compression << " chunk @ slice " << chunk.first.first_slice
		      << " + " << print_hex(offsets.data_offset) << " + " << print_hex(chunk.first.offset)
		      << ']');
		
		if(chunk.first.encrypted) {
			log_warning << "skipping encrypted chunk (unsupported)";
			continue;
		}
		
		stream::chunk_reader::pointer chunk_source;
		if(o.extract || o.test) {
			chunk_source = stream::chunk_reader::get(*slice_reader, chunk.first);
		}
		boost::uint64_t offset = 0;
		
		BOOST_FOREACH(const Files::value_type & location, chunk.second) {
			const stream::file & file = location.first;
			
			// Convert output filenames
			typedef std::pair<std::string, size_t> file_t;
			std::vector<file_t> output_names;
			for(size_t i = 0; i < files_for_location[location.second].size(); i++) {
				
				size_t file_i = files_for_location[location.second][i];
				
				if(!o.language.empty() && !info.files[file_i].languages.empty()) {
					if(!setup::expression_match(o.language, info.files[file_i].languages)) {
						continue;
					}
				}
				
				if(!info.files[file_i].destination.empty()) {
					std::string path = o.filenames.convert(info.files[file_i].destination);
					if(!path.empty()) {
						output_names.push_back(std::make_pair(path, file_i));
					}
				}
			}
			
			if(output_names.empty()) {
				extract_progress.update(location.first.size);
				continue;
			}
			
			// Print filename and size
			if(o.list) {
				
				extract_progress.clear();
				
				if(!o.silent) {
					
					std::cout << " - ";
					bool named = false;
					BOOST_FOREACH(const file_t & path, output_names) {
						if(named) {
							std::cout << ", ";
						}
						std::cout << '"' << color::white << path.first << color::reset << '"';
						if(!info.files[path.second].languages.empty()) {
							std::cout << " [" << color::green << info.files[path.second].languages
							          << color::reset << "]";
						}
						named = true;
					}
					if(!named) {
						std::cout << color::white << "unnamed file" << color::reset;
					}
					if(!o.quiet) {
						if(logger::debug) {
							std::cout << " @ " << print_hex(file.offset);
						}
						std::cout << " (" << color::dim_cyan << print_bytes(file.size)
						          << color::reset << ")";
					}
					std::cout << '\n';
					
				} else {
					BOOST_FOREACH(const file_t & path, output_names) {
						std::cout << color::white << path.first << color::reset << '\n';
					}
				}
				
				bool updated = extract_progress.update(0, true);
				if(!updated && (o.extract || o.test)) {
					std::cout.flush();
				}
				
			}
			
			if(!o.extract && !o.test) {
				continue;
			}
			
			// Seek to the correct position within the chunk
			if(file.offset < offset) {
				log_error << "bad offset";
				throw std::runtime_error("unexpected error");
			}
			if(file.offset > offset) {
				debug("discarding " << print_bytes(file.offset - offset));
				util::discard(*chunk_source, file.offset - offset);
			}
			offset = file.offset + file.size;
			
			crypto::checksum checksum;
			
			// Open input file
			stream::file_reader::pointer file_source;
			file_source = stream::file_reader::get(*chunk_source, file, &checksum);
			
			// Open output files
			boost::ptr_vector<file_output> output;
			if(!o.test) {
				output.reserve(output_names.size());
				BOOST_FOREACH(const file_t & path, output_names) {
					output.push_back(new file_output(o.output_dir / path.first));
				}
			}
			
			// Copy data
			while(!file_source->eof()) {
				char buffer[8192 * 10];
				std::streamsize buffer_size = std::streamsize(ARRAY_SIZE(buffer));
				std::streamsize n = file_source->read(buffer, buffer_size).gcount();
				if(n > 0) {
					BOOST_FOREACH(file_output & out, output) {
						out.stream.write(buffer, n);
						if(out.stream.fail()) {
							throw std::runtime_error("error writing file \""
							                         + out.name.string() + '"');
						}
					}
					extract_progress.update(boost::uint64_t(n));
				}
			}
			
			// Adjust file timestamps
			if(o.preserve_file_times) {
				const setup::data_entry & data = info.data_entries[location.second];
				util::time filetime = data.timestamp;
				if(o.local_timestamps && !(data.options & data.TimeStampInUTC)) {
					filetime = util::to_local_time(filetime);
				}
				BOOST_FOREACH(file_output & out, output) {
					out.stream.close();
					if(!util::set_file_time(out.name, filetime, data.timestamp_nsec)) {
						log_warning << "error setting timestamp on file " << out.name;
					}
				}
			}
			
			// Verify checksums
			if(checksum != file.checksum) {
				log_warning << "checksum mismatch:\n"
				            << "actual:   " << checksum << '\n'
				            << "expected: " << file.checksum;
				if(o.test) {
					throw std::runtime_error("integrity test failed");
				}
			}
		}
	}
	
	extract_progress.clear();
}


int main(int argc, char * argv[]) {
	
	::options o;
	
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
	;
	
	po::options_description filter("Modifiers");
	filter.add_options()
		("dump", "Dump contents without converting filenames")
		("lowercase,L", "Convert extracted filenames to lower-case")
		("language", po::value<std::string>(), "Extract files for the given language")
		("timestamps,T", po::value<std::string>(), "Timezone for file times or \"local\" or \"none\"")
		("output-dir,d", po::value<fs::path>(), "Extract files into the given directory")
	;
	
	po::options_description io("Display options");
	io.add_options()
		("quiet,q", "Output less information")
		("silent,s", "Output only error/warning information")
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
	options_desc.add(generic).add(action).add(filter).add(io).add(hidden);
	
	po::options_description visible;
	visible.add(generic).add(action).add(filter).add(io);
	
	po::positional_options_description p;
	p.add("setup-files", -1);
	
	po::variables_map options;
	
	// Parse the command-line.
	try {
		po::store(po::command_line_parser(argc, argv).options(options_desc).positional(p).run(),
		          options);
		po::notify(options);
	} catch(po::error & e) {
		std::cerr << "Error parsing command-line: " << e.what() << "\n\n";
		print_help(get_command(argv[0]), visible);
		return ExitUserError;
	}
	
	// Verbosity settings.
	o.silent = (options.count("silent") != 0);
	o.quiet = o.silent || options.count("quiet");
	logger::quiet = o.quiet;
#ifdef DEBUG
	if(options.count("debug")) {
		logger::debug = true;
	}
#endif
	
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
	bool explicit_action = o.list || o.test || o.extract;
	if(!explicit_action) {
		o.extract = true;
	}
	if(o.extract + o.test > 1) {
		log_error << "cannot specify multiple actions";
		return ExitUserError;
	}
	if(!o.extract && !o.test) {
		progress::set_enabled(false);
	}
	if(!o.silent) {
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
	
	if(!options.count("setup-files") != 0) {
		if(!o.silent) {
			std::cout << get_command(argv[0]) << ": no input files specified\n";
			std::cout << "Try the --help (-h) option for usage information.\n";
		}
		return ExitSuccess;
	}
	
	{
		po::variables_map::const_iterator i = options.find("output-dir");
		if(i != options.end()) {
			o.output_dir = i->second.as<fs::path>();
			try {
				if(!fs::exists(o.output_dir)) {
					fs::create_directory(o.output_dir);
				}
			} catch(...) {
				log_error << "could not create output directory " << o.output_dir;
				return ExitDataError;
			}
		}
	}
	
	const std::vector<std::string> & files = options["setup-files"]
	                                         .as< std::vector<std::string> >();
	
	try {
		BOOST_FOREACH(const std::string & file, files) {
			process_file(file, o);
		}
	} catch(std::ios_base::failure e) {
		log_error << "stream error: " << e.what();
	} catch(std::runtime_error e) {
		log_error << e.what();
	} catch(setup::version_error e) {
		log_error << "not a supported Inno Setup installer";
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
