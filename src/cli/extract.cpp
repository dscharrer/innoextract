/*
 * Copyright (C) 2011-2014 Daniel Scharrer
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

#include "cli/extract.hpp"

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <map>
#include <sstream>
#include <vector>

#include <boost/foreach.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/range/size.hpp>

#include "cli/debug.hpp"
#include "cli/gog.hpp"

#include "loader/offsets.hpp"

#include "setup/data.hpp"
#include "setup/expression.hpp"
#include "setup/file.hpp"
#include "setup/info.hpp"
#include "setup/language.hpp"

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

namespace fs = boost::filesystem;

namespace {

struct file_output {
	
	fs::path name;
	util::ofstream stream;
	
	explicit file_output(const fs::path & file) : name(file) {
		try {
			fs::create_directories(name.parent_path());
		} catch(...) {
			throw std::runtime_error("Could not create directories for \""
			                         + name.string() + '"');
		}
		stream.open(name, std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
		if(!stream.is_open()) {
			throw std::runtime_error("Coul not open output file \"" + name.string() + '"');
		}
	}
	
};

static bool probe_bin_file(const fs::path & file) {
	try {
		if(!fs::is_regular_file(file)) {
			return false;
		}
	} catch(...) {
		return false;
	}
	log_warning << file << " is not part of the installer!";
	return true;
}

static void probe_bin_files(const fs::path & dir, const std::string & basename,
                            size_t start, size_t format) {
	for(size_t i = start;; i++) {
		if(!probe_bin_file(dir / stream::slice_reader::slice_filename(basename, i, format))) {
			break;
		}
	}
}

class path_filter {
	
	typedef std::pair<bool, std::string> Filter;
	std::vector<Filter> includes;
	
public:
	
	explicit path_filter(const extract_options & o) {
		BOOST_FOREACH(const std::string & include, o.include) {
			if(!include.empty() && include[0] == setup::path_sep) {
				includes.push_back(Filter(true, include + setup::path_sep));
			} else {
				includes.push_back(Filter(false, setup::path_sep + include + setup::path_sep));
			}
		}
	}
	
	bool match(const std::string & path) const {
		
		if(includes.empty()) {
			return true;
		}
		
		BOOST_FOREACH(const Filter & i, includes) {
			if(i.first) {
				if(!i.second.compare(1, i.second.size() - 1,
				                     path + setup::path_sep, 0, i.second.size() - 1)) {
					return true;
					break;
				}
			} else {
				if((setup::path_sep + path + setup::path_sep).find(i.second) != std::string::npos) {
					return true;
					break;
				}
			}
		}
		
		return false;
	}
	
};

static void print_filter_info(const setup::item & item) {
	
	if(!item.languages.empty()) {
		std::cout << " [" << color::green << item.languages << color::reset << "]";
	}
	
}

static void print_size_info(const stream::file & file) {
	
	if(logger::debug) {
		std::cout << " @ " << print_hex(file.offset);
	}
	
	std::cout << " (" << color::dim_cyan << print_bytes(file.size) << color::reset << ")";
}

} // anonymous namespace

void process_file(const fs::path & file, const extract_options & o) {
	
	bool is_directory;
	try {
		is_directory = fs::is_directory(file);
	} catch(...) {
		throw std::runtime_error("Could not open file \"" + file.string()
		                         + "\": access denied");
	}
	if(is_directory) {
		throw std::runtime_error("Input file \"" + file.string() + "\" is a directory!");
	}
	
	util::ifstream ifs(file, std::ios_base::in | std::ios_base::binary);
	if(!ifs.is_open()) {
		throw std::runtime_error("Could not open file \"" + file.string() + '"');
	}
	
	loader::offsets offsets;
	offsets.load(ifs);
	
#ifdef DEBUG
	if(logger::debug) {
		print_offsets(offsets);
		std::cout << '\n';
	}
#endif
	
	setup::info::entry_types entries = 0;
	if(o.list || o.test || o.extract) {
		entries |= setup::info::Files;
		entries |= setup::info::DataEntries;
	}
	if(o.list_languages) {
		entries |= setup::info::Languages;
	}
	if(o.gog_game_id) {
		entries |= setup::info::RegistryEntries;
	}
#ifdef DEBUG
	if(logger::debug) {
		entries = setup::info::entry_types::all();
	}
#endif
	
	ifs.seekg(offsets.header_offset);
	setup::info info;
	try {
		info.load(ifs, entries);
	} catch(const std::ios_base::failure & e) {
		std::ostringstream oss;
		oss << "Stream error while parsing setup headers!\n";
		oss << " ├─ detected setup version: " << info.version << '\n';
		oss << " └─ error reason: " << e.what();
		throw format_error(oss.str());
	}
	
	if(!o.quiet) {
		const std::string & name = info.header.app_versioned_name.empty()
		                           ? info.header.app_name : info.header.app_versioned_name;
		const char * verb = "Inspecting";
		if(o.extract) {
			verb = "Extracting";
		} else if(o.test) {
			verb = "Testing";
		} else if(o.list) {
			verb = "Listing";
		}
		std::cout << verb << " \"" << color::green << name << color::reset
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
	
	bool multiple_sections = (o.list_languages + o.gog_game_id + o.list > 1);
	if(!o.quiet && multiple_sections) {
		std::cout << '\n';
	}
	
	if(o.list_languages) {
		if(o.silent) {
			BOOST_FOREACH(const setup::language_entry & language, info.languages) {
				std::cout << language.name <<' ' << language.language_name << '\n';
			}
		} else {
			if(multiple_sections) {
				std::cout << "Languages:\n";
			}
			BOOST_FOREACH(const setup::language_entry & language, info.languages) {
				std::cout << " - " << color::green << language.name << color::reset;
				std::cout << ": " << color::white << language.language_name << color::reset << '\n';
			}
			if(info.languages.empty()) {
				std::cout << " (none)\n";
			}
		}
		if((o.silent || !o.quiet) && multiple_sections) {
			std::cout << '\n';
		}
	}
	
	if(o.gog_game_id) {
		std::string id = gog::get_game_id(info);
		if(id.empty()) {
			if(!o.quiet) {
				std::cout << "No GOG.com game ID found!\n";
			}
		} else if(!o.silent) {
			std::cout << "GOG.com game ID is " << color::cyan << id << color::reset << '\n';
		} else {
			std::cout << id;
		}
		if((o.silent || !o.quiet) && multiple_sections) {
			std::cout << '\n';
		}
	}
	
	if(!o.list && !o.test && !o.extract) {
		return;
	}
	
	if(!o.silent && multiple_sections) {
		std::cout << "Files:\n";
	}
	
	path_filter includes(o);
	
	std::vector< std::vector<size_t> > files_for_location;
	files_for_location.resize(info.data_entries.size());
	for(size_t i = 0; i < info.files.size(); i++) {
		if(info.files[i].location < files_for_location.size()) {
			files_for_location[info.files[i].location].push_back(i);
		}
	}
	
	boost::uint64_t total_size = 0;
	size_t max_slice = 0;
	
	typedef std::map<stream::file, size_t> Files;
	typedef std::map<stream::chunk, Files> Chunks;
	Chunks chunks;
	for(size_t i = 0; i < info.data_entries.size(); i++) {
		setup::data_entry & location = info.data_entries[i];
		if(!offsets.data_offset) {
			max_slice = std::max(max_slice, location.chunk.first_slice);
			max_slice = std::max(max_slice, location.chunk.last_slice);
		}
		if(files_for_location[i].empty()) {
			continue;
		}
		if(location.chunk.compression == stream::UnknownCompression) {
			location.chunk.compression = info.header.compression;
		}
		chunks[location.chunk][location.file] = i;
		total_size += location.file.size;
	}
	
	fs::path dir = file.parent_path();
	std::string basename = util::as_string(file.stem());
	
	boost::scoped_ptr<stream::slice_reader> slice_reader;
	if(o.extract || o.test) {
		if(offsets.data_offset) {
			slice_reader.reset(new stream::slice_reader(&ifs, offsets.data_offset));
		} else {
			slice_reader.reset(new stream::slice_reader(dir, basename, info.header.slices_per_disk));
		}
	}
	
	progress extract_progress(total_size);
	
	BOOST_FOREACH(const Chunks::value_type & chunk, chunks) {
		
		debug("[starting " << chunk.first.compression << " chunk @ slice " << chunk.first.first_slice
		      << " + " << print_hex(offsets.data_offset) << " + " << print_hex(chunk.first.offset)
		      << ']');
		
		if(chunk.first.encrypted) {
			log_warning << "Skipping encrypted chunk (unsupported)";
		}
		
		stream::chunk_reader::pointer chunk_source;
		if((o.extract || o.test) && !chunk.first.encrypted) {
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
					if(!path.empty() && includes.match(path)) {
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
						if(chunk.first.encrypted) {
							std::cout << '"' << color::dim_yellow << path.first << color::reset << '"';
						} else {
							std::cout << '"' << color::white << path.first << color::reset << '"';
						}
						print_filter_info(info.files[path.second]);
						named = true;
					}
					if(!named) {
						std::cout << color::white << "unnamed file" << color::reset;
					}
					if(!o.quiet) {
						print_size_info(file);
					}
					if(chunk.first.encrypted) {
						std::cout << " - encrypted";
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
			
			if((!o.extract && !o.test) || chunk.first.encrypted) {
				continue;
			}
			
			// Seek to the correct position within the chunk
			if(file.offset < offset) {
				std::ostringstream oss;
				oss << "Bad offset while extracting files: file start (" << file.offset
				    << ") is before end of previous file (" << offset << ")!";
				throw format_error(oss.str());
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
					try {
						output.push_back(new file_output(o.output_dir / path.first));
					} catch(boost::bad_pointer &) {
						// should never happen
						std::terminate();
					}
				}
			}
			
			// Copy data
			while(!file_source->eof()) {
				char buffer[8192 * 10];
				std::streamsize buffer_size = std::streamsize(boost::size(buffer));
				std::streamsize n = file_source->read(buffer, buffer_size).gcount();
				if(n > 0) {
					BOOST_FOREACH(file_output & out, output) {
						out.stream.write(buffer, n);
						if(out.stream.fail()) {
							throw std::runtime_error("Error writing file \""
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
						log_warning << "Error setting timestamp on file " << out.name;
					}
				}
			}
			
			// Verify checksums
			if(checksum != file.checksum) {
				log_warning << "Checksum mismatch:\n"
				            << " ├─ actual:   " << checksum << '\n'
				            << " └─ expected: " << file.checksum;
				if(o.test) {
					throw std::runtime_error("Integrity test failed!");
				}
			}
		}
	}
	
	extract_progress.clear();
	
	if(o.warn_unused) {
		probe_bin_file(dir / (basename + ".bin"));
		probe_bin_file(dir / (basename + "-0" + ".bin"));
		size_t slice =  0;
		size_t format = 1;
		if(!offsets.data_offset && info.header.slices_per_disk == 1) {
			slice = max_slice + 1;
		}
		probe_bin_files(dir, basename, slice, format);
		slice = 0;
		format = 2;
		if(!offsets.data_offset && info.header.slices_per_disk != 1) {
			slice = max_slice + 1;
			format = info.header.slices_per_disk;
		}
		probe_bin_files(dir, basename, slice, format);
	}
	
}
