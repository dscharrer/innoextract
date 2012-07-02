/*
 * Copyright (C) 2011-2012 Daniel Scharrer
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

#include <fstream>
#include <iostream>
#include <iomanip>
#include <string>
#include <algorithm>
#include <cstring>
#include <vector>
#include <map>
#include <cctype>

#include <boost/foreach.hpp>
#include <boost/make_shared.hpp>
#include <boost/program_options.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/filesystem/operations.hpp>

#include "cli/debug.hpp"
#include "cli/version.hpp"

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
#include "util/load.hpp"
#include "util/log.hpp"
#include "util/output.hpp"

using std::cout;
using std::string;
using std::endl;
using std::setw;
using std::setfill;

namespace io = boost::iostreams;
namespace fs = boost::filesystem;
namespace po = boost::program_options;

static void print_version() {
	std::cout << color::white << innoextract_version << color::reset
#ifdef DEBUG
	          << " (with debug output)"
#endif
	          << '\n';
	std::cout << "Extracts installers created by " << color::cyan
	          << innosetup_versions << color::reset << '\n';
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
	std::cout << color::white << innoextract_version << color::reset
	          << ' ' << innoextract_copyright << '\n';
	std::cout << "This is free software with absolutely no warranty.\n";
}

static void print_license() {
	
	std::cout << color::white << innoextract_version << color::reset
	          << ' ' << innoextract_copyright << '\n';
	std::cout << '\n';
	std::cout << "This software is provided 'as-is', without any express or implied\n"
	             "warranty.  In no event will the author(s) be held liable for any damages\n"
	             "arising from the use of this software.\n"
	             "\n"
	             "Permission is granted to anyone to use this software for any purpose,\n"
	             "including commercial applications, and to alter it and redistribute it\n"
	             "freely, subject to the following restrictions:\n"
	             "\n"
	             "1. The origin of this software must not be misrepresented; you must not\n"
	             "   claim that you wrote the original software. If you use this software\n"
	             "   in a product, an acknowledgment in the product documentation would be\n"
	             "   appreciated but is not required.\n"
	             "2. Altered source versions must be plainly marked as such, and must not be\n"
	             "   misrepresented as being the original software.\n"
	             "3. This notice may not be removed or altered from any source distribution.\n"
	;
}

struct options {
	
	bool silent;
	
	bool dump;
	bool list;
	bool test;
	
	std::string language;
	
	setup::filename_map filenames;
	
};

static void process_file(const fs::path & file, const options & o) {
	
	fs::ifstream ifs(file, std::ios_base::in | std::ios_base::binary | std::ios_base::ate);
	if(!ifs.is_open()) {
		throw std::runtime_error("error opening file \"" + file.string() + '"');
	}
	
	loader::offsets offsets;
	offsets.load(ifs);
	
	cout << std::boolalpha;
	
#ifdef DEBUG
	if(logger::debug) {
		print_offsets(offsets);
		cout << '\n';
	}
#endif
	
	ifs.seekg(offsets.header_offset);
	setup::info info;
	info.load(ifs);
	
	if(!logger::quiet) {
		const std::string & name = info.header.app_versioned_name.empty()
		                           ? info.header.app_name : info.header.app_versioned_name;
		cout << "Extracting \"" << color::green << name << color::reset
		     << "\" - setup data version " << color::white << info.version << color::reset
		     << std::endl;
	}
	
#ifdef DEBUG
	if(logger::debug) {
		cout << '\n';
		print_info(info);
		cout << '\n';
	}
#endif
	
	uint64_t total_size = 0;
	
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
	if(!o.list) {
		if(offsets.data_offset) {
			slice_reader.reset(new stream::slice_reader(file, offsets.data_offset));
		} else {
			slice_reader.reset(new stream::slice_reader(file.parent_path(), file.stem(),
			                                            info.header.slices_per_disk));
		}
	}
	
	progress extract_progress(total_size);
	
	BOOST_FOREACH(const Chunks::value_type & chunk, chunks) {
		
		debug("[starting " << chunk.first.compression << " chunk @ " << chunk.first.first_slice
		      << " + " << print_hex(offsets.data_offset) << " + " << print_hex(chunk.first.offset)
		      << ']');
		
		stream::chunk_reader::pointer chunk_source;
		if(!o.list) {
			chunk_source = stream::chunk_reader::get(*slice_reader, chunk.first);
		}
		
		uint64_t offset = 0;
		
		BOOST_FOREACH(const Files::value_type & location, chunk.second) {
			const stream::file & file = location.first;
			
			// Convert output filenames
			typedef std::pair<fs::path, size_t> file_t;
			std::vector<file_t> output_names;
			for(size_t i = 0; i < files_for_location[location.second].size(); i++) {
				
				size_t file_i = files_for_location[location.second][i];
				
				if(!o.language.empty() && !info.files[file_i].languages.empty()) {
					if(!setup::expression_match(o.language, info.files[file_i].languages)) {
						continue;
					}
				}
				
				if(!info.files[file_i].destination.empty()) {
					fs::path path;
					if(o.dump) {
						std::string file = info.files[file_i].destination;
						if(o.filenames.lowercase) {
							std::transform(file.begin(), file.end(), file.begin(), ::tolower);
						}
						path = file;
					} else {
						path = o.filenames.convert(info.files[file_i].destination);
					}
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
			if(!o.silent) {
				
				progress::clear();
				
				std::cout << " - ";
				bool named = false;
				BOOST_FOREACH(const file_t & path, output_names) {
					if(named) {
						std::cout << ", ";
					}
					std::cout << '"' << color::white << path.first.string() << color::reset << '"';
					if(!info.files[path.second].languages.empty()) {
						std::cout << " [" << color::green << info.files[path.second].languages
						          << color::reset << "]";
					}
					named = true;
				}
				if(!named) {
					std::cout << color::white << "unnamed file" << color::reset;
				}
				if(!logger::quiet) {
				#ifdef DEBUG
					std::cout << " @ " << print_hex(file.offset);
				#endif
				std::cout << " (" << color::dim_cyan << print_bytes(file.size)
				          << color::reset << ")";
				}
				std::cout << '\n';
				
				if(!o.list) {
					extract_progress.update(0, true);
				}
			}
			
			if(o.list) {
				continue;
			}
			
			// Seek to the correct position within the chunk
			if(file.offset < offset) {
				log_error << "bad offset";
				throw std::runtime_error("unexpected error");
			}
			if(file.offset > offset) {
				debug("discarding " << print_bytes(file.offset - offset));
				discard(*chunk_source, file.offset - offset);
			}
			offset = file.offset + file.size;
			
			crypto::checksum checksum;
			
			// Open input file
			stream::file_reader::pointer file_source;
			file_source = stream::file_reader::get(*chunk_source, file, &checksum);
			
			// Open output files
			boost::ptr_vector<fs::ofstream> output;
			if(!o.test) {
				output.reserve(output_names.size());
				BOOST_FOREACH(const file_t & path, output_names) {
					try {
						fs::create_directories(path.first.parent_path());
					} catch(...) {
						throw std::runtime_error("error creating directories for \""
						                         + path.first.string() + '"');
					}
					output.push_back(new fs::ofstream(path.first));
					if(!output.back().is_open()) {
						throw std::runtime_error("error opening output file \""
						                        + path.first.string() + '"');
					}
				}
			}
			
			// Copy data
			while(!file_source->eof()) {
				char buffer[8192 * 10];
				std::streamsize n = file_source->read(buffer, ARRAY_SIZE(buffer)).gcount();
				if(n > 0) {
					
					BOOST_FOREACH(fs::ofstream & ofs, output) {
						ofs.write(buffer, n);
					}
					
					extract_progress.update(uint64_t(n));
				}
			}
			
			if(checksum != file.checksum) {
				log_warning << "checksum mismatch:\n"
				            << "actual:   " << checksum << '\n'
				            << "expected: " << file.checksum;
				if(o.test) {
					throw new std::runtime_error("integrity test failed");
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
		("version,v", "Print the version information")
		("license", "Show license information")
	;
	
	po::options_description action("Actions");
	action.add_options()
		("test,t", "Only verify checksums, don't write anything")
		("extract,e", "Extract files (default action)")
		("list,l", "Only list files, don't write anything")
	;
	
	po::options_description filter("Filters");
	filter.add_options()
		("dump", "Dump contents without converting filenames")
		("lowercase,L", "Convert extracted filenames to lower-case")
		("language", po::value<std::string>(), "Extract files for the given language")
	;
	
	po::options_description io("I/O options");
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
		("setup-files", po::value< std::vector<string> >(), "Setup files to be extracted")
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
		print_help(argv[0], visible);
		return 1;
	}
	
	// Verbosity settings.
	o.silent = options.count("silent");
	logger::quiet = o.silent || options.count("quiet");
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
	if(options.count("help")) {
		print_help(argv[0], visible);
		return 0;
	}
	
	// License output
	if(options.count("license")) {
		print_license();
		return 0;
	}
	
	// Main action.
	o.list = options.count("list");
	bool extract = options.count("extract");
	o.test = options.count("test");
	bool explicit_action = o.list || extract || o.test;
	if(!explicit_action) {
		extract = true;
	}
	if(o.list + extract + o.test > 1) {
		log_error << "cannot specify multiple actions";
		return 0;
	}
	
	// Additional actions.
	o.dump = options.count("dump");
	o.filenames.set_lowercase(options.count("lowercase"));
	
	// List version.
	if(options.count("version")) {
		print_version();
		if(!explicit_action) {
			return 0;
		}
	}
	
	po::variables_map::const_iterator i = options.find("language");
	if(i != options.end()) {
		o.language = i->second.as<std::string>();
	}
	
	if(!options.count("setup-files")) {
		if(!o.silent) {
			std::cout << "no input files specified\n";
		}
		return 0;
	}
	
	const std::vector<string> & files = options["setup-files"].as< std::vector<string> >();
	
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
		std::cout << color::green << "Done" << color::reset << std::dec;
		if(logger::total_errors || logger::total_warnings) {
			std::cout << " with ";
			if(logger::total_errors) {
				std::cout << color::red << logger::total_errors
				          << ((logger::total_errors == 1) ? " error" : " errors")
				          << color::reset;
			}
			if(logger::total_errors && logger::total_warnings) {
				std::cout << " and ";
			}
			if(logger::total_warnings) {
				std::cout << color::yellow << logger::total_warnings
				          << ((logger::total_warnings == 1) ? " warning" : " warnings")
				          << color::reset;
			}
		}
		std::cout << '.' << std::endl;
	}
	
	return logger::total_errors == 0 ? 0 : 1;
}
