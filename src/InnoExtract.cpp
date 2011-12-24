
#include <fstream>
#include <iostream>
#include <iomanip>
#include <string>
#include <algorithm>
#include <cstring>
#include <vector>
#include <bitset>
#include <ctime>
#include <map>

#include <boost/shared_ptr.hpp>
#include <boost/foreach.hpp>
#include <boost/ref.hpp>
#include <boost/make_shared.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/iostreams/copy.hpp>

#include "version.hpp"

#include "cli/debug.hpp"

#include "loader/offsets.hpp"

#include "setup/data.hpp"
#include "setup/file.hpp"
#include "setup/info.hpp"
#include "setup/version.hpp"

#include "stream/chunk.hpp"
#include "stream/file.hpp"
#include "stream/slice.hpp"

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

int main(int argc, char * argv[]) {
	
	color::init();
	
	// logger::debug = true;
	
	if(argc <= 1) {
		cout << "usage: innoextract <Inno Setup installer>" << endl;
		return 1;
	}
	
	if(!strcmp(argv[1], "-v") || !strcmp(argv[1], "--version")) {
		cout << innoextract_version << endl;
		return 1;
	}
	
	std::ifstream ifs(argv[1], std::ios_base::in | std::ios_base::binary | std::ios_base::ate);
	if(!ifs.is_open()) {
		log_error << "error opening file \"" << argv[1] << '"';
		return 1;
	}
	
	loader::offsets offsets;
	offsets.load(ifs);
	
	cout << std::boolalpha;
	
	if(logger::debug) {
		print_offsets(offsets);
		cout << '\n';
	}
	
	ifs.seekg(offsets.header_offset);
	setup::info info;
	info.load(ifs);
	
	const std::string & name = info.header.app_versioned_name.empty()
	                           ? info.header.app_name : info.header.app_versioned_name;
	cout << "Extracting \"" << color::green << name << color::reset
	     << "\" - setup data version " << color::white << info.version << color::reset << std::endl;
	
	if(logger::debug) {
		cout << '\n';
		print_info(info);
	}
	
	cout << '\n';
	
	std::vector<std::vector<size_t> > files_for_location;
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
	}
	
	boost::shared_ptr<stream::slice_reader> slice_reader;
	
	if(offsets.data_offset) {
		slice_reader = boost::make_shared<stream::slice_reader>(argv[1], offsets.data_offset);
	} else {
		fs::path path(argv[1]);
		slice_reader = boost::make_shared<stream::slice_reader>(path.parent_path().string() + '/',
		                                                        path.stem().string(),
		                                                        info.header.slices_per_disk);
	}
	
	try {
	
	BOOST_FOREACH(const Chunks::value_type & chunk, chunks) {
		
		cout << "[starting " << chunk.first.compression << " chunk @ " << chunk.first.first_slice
		     << " + " << print_hex(offsets.data_offset) << " + " << print_hex(chunk.first.offset)
		     << ']' << std::endl;
		
		stream::chunk_reader::pointer chunk_source;
		chunk_source = stream::chunk_reader::get(*slice_reader, chunk.first);
		
		uint64_t offset = 0;
		
		BOOST_FOREACH(const Files::value_type & location, chunk.second) {
			const stream::file & file = location.first;
			
			if(file.offset < offset) {
				log_error << "bad offset";
				return 1;
			}
			
			if(file.offset > offset) {
				std::cout << "discarding " << print_bytes(file.offset - offset) << std::endl;
				discard(*chunk_source, file.offset - offset);
			}
			offset = file.offset + file.size;
			
			std::cout << "-> reading ";
			bool named = false;
			BOOST_FOREACH(size_t file_i, files_for_location[location.second]) {
				if(!info.files[file_i].destination.empty()) {
					std::cout << '"' << info.files[file_i].destination << '"';
					named = true;
					break;
				}
			}
			if(!named) {
				std::cout << "unnamed file";
			}
			std::cout << " @ " << print_hex(file.offset)
			          << " (" << print_bytes(file.size) << ')' << std::endl;
			
			crypto::checksum checksum;
			
			stream::file_reader::pointer file_source;
			file_source = stream::file_reader::get(*chunk_source, file, &checksum);
			
			BOOST_FOREACH(size_t file_i, files_for_location[location.second]) {
				if(!info.files[file_i].destination.empty()) {
					std::ofstream ofs(info.files[file_i].destination.c_str());
					
					progress extract_progress(file.size);
					
					char buffer[8192 * 10];
					
					while(!file_source->eof()) {
						std::streamsize n = file_source->read(buffer, ARRAY_SIZE(buffer)).gcount();
						if(n > 0) {
							ofs.write(buffer, n);
							extract_progress.update(uint64_t(n));
						}
					}
					
					extract_progress.clear();
					
					break; // TODO ...
				}
			}
			
			if(checksum != file.checksum) {
				log_warning << "checksum mismatch:";
				log_warning << "actual:   " << checksum;
				log_warning << "expected: " << file.checksum;
			}
		}
	}
	
	} catch(std::ios_base::failure e) {
		log_error << e.what();
	}
	
	std::cout << color::green << "Done" << color::reset << std::dec;
	
	if(logger::total_errors || logger::total_warnings) {
		std::cout << " with ";
		if(logger::total_errors) {
			std::cout << color::red << logger::total_errors << " errors" << color::reset;
		}
		if(logger::total_errors && logger::total_warnings) {
			std::cout << " and ";
		}
		if(logger::total_warnings) {
			std::cout << color::yellow << logger::total_warnings << " warnings" << color::reset;
		}
	}
	
	std::cout << '.' << std::endl;
	
	return 0;
}
