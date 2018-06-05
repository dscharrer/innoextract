/*
 * Copyright (C) 2018 Daniel Scharrer
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

#include "cli/goggalaxy.hpp"

#include <string>
#include <vector>

#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>

#include "setup/data.hpp"
#include "setup/file.hpp"
#include "setup/info.hpp"

#include "util/log.hpp"

namespace gog {

namespace {

std::vector<std::string> parse_function_call(const std::string & code, const std::string & name) {
	
	std::vector<std::string> arguments;
	if(code.empty()) {
		return arguments;
	}
	
	const char whitespace[] = " \t\r\n";
	const char separator[] = " \t\r\n(),'";
	
	size_t start = code.find_first_not_of(whitespace);
	if(start == std::string::npos) {
		return arguments;
	}
	
	size_t end = code.find_first_of(separator, start);
	if(end == std::string::npos) {
		return arguments;
	}
	
	size_t parenthesis = code.find_first_not_of(whitespace, end);
	if(parenthesis == std::string::npos || code[parenthesis] != '(') {
		return arguments;
	}
	
	if(end - start != name.length() || code.compare(start, end - start, name) != 0) {
		return arguments;
	}
	
	size_t p = parenthesis + 1;
	while(true) {
		
		p = code.find_first_not_of(whitespace, p);
		if(p == std::string::npos) {
			log_warning << "Error parsing function call: " << code;
			return arguments;
		}
		
		arguments.resize(arguments.size() + 1);
		
		if(code[p] == '\'') {
			p++;
			while(true) {
				size_t string_end = code.find('\'', p);
				arguments.back() += code.substr(p, string_end - p);
				if(string_end == std::string::npos || string_end + 1 == code.size()) {
					log_warning << "Error parsing function call: " << code;
					return arguments;
				}
				p = string_end + 1;
				if(code[p] == '\'') {
					arguments.back() += '\'';
					p++;
				} else {
					break;
				}
			}
		} else {
			size_t token_end = code.find_first_of(separator, p);
			arguments.back() = code.substr(p, token_end - p);
			if(token_end == std::string::npos || token_end == code.size()) {
				log_warning << "Error parsing function call: " << code;
				return arguments;
			}
			p = token_end;
		}
		
		p = code.find_first_not_of(whitespace, p);
		if(p == std::string::npos) {
			log_warning << "Error parsing function call: " << code;
			return arguments;
		}
		
		if(code[p] == ')') {
			break;
		} else if(code[p] == ',') {
			p++;
		} else {
			log_warning << "Error parsing function call: " << code;
			return arguments;
		}
		
	}
	
	p++;
	if(p != code.size()) {
		p = code.find_first_not_of(whitespace, p);
		if(p != std::string::npos) {
			if(code[p] != ';' || code.find_first_not_of(whitespace, p + 1) != std::string::npos) {
				log_warning << "Error parsing function call: " << code;
			}
		}
	}
	
	return arguments;
}

} // anonymous namespace

void parse_galaxy_files(setup::info & info) {
	
	setup::file_entry * file_start = NULL;
	size_t remaining_parts = 0;
	
	BOOST_FOREACH(setup::file_entry & file, info.files) {
		
		// Multi-part file info: file checksum, filename, part count
		std::vector<std::string> start_info = parse_function_call(file.before_install, "before_install");
		if(start_info.empty()) {
			start_info = parse_function_call(file.before_install, "before_install_dependency");
		}
		if(!start_info.empty()) {
			
			if(remaining_parts != 0) {
				log_warning << "Incomplete GOG Galaxy file " << file_start->destination;
				remaining_parts = 0;
			}
			
			// Recover the original filename - parts are named after the MD5 hash of their contents
			if(start_info.size() >= 2 && !start_info[1].empty()) {
				file.destination = start_info[1];
			}
			
			if(start_info.size() < 3) {
				log_warning << "Missing part count for GOG Galaxy file " << file.destination;
				remaining_parts = 1;
			} else {
				try {
					remaining_parts = boost::lexical_cast<size_t>(start_info[2]);
					if(remaining_parts == 0) {
						remaining_parts = 1;
					}
					file_start = &file;
				} catch(...) {
					log_warning << "Could not parse part count for GOG Galaxy file " << file.destination
					            << ": " << start_info[2];
				}
			}
			
		}
		
		// File part ifo: part checksum, compressed part size, uncompressed part size
		std::vector<std::string> part_info = parse_function_call(file.after_install, "after_install");
		if(part_info.empty()) {
			part_info = parse_function_call(file.after_install, "after_install_dependency");
		}
		if(!part_info.empty()) {
			if(remaining_parts == 0) {
				log_warning << "Missing file start for GOG Galaxy file part " << file.destination;
			} else if(file.location > info.data_entries.size()) {
				log_warning << "Invalid data location for GOG Galaxy file part " << file.destination;
				remaining_parts = 0;
			} else if(part_info.size() < 3) {
				log_warning << "Missing size for GOG Galaxy file part " << file.destination;
				remaining_parts = 0;
			} else {
				
				remaining_parts--;
				
				setup::data_entry & data = info.data_entries[file.location];
				
				// Ignore file part MD5 checksum, setup already contains a better one for the deflated data
				
				try {
					boost::uint64_t compressed_size = boost::lexical_cast<boost::uint64_t>(part_info[1]);
					if(data.file.size != compressed_size) {
						log_warning << "Unexpected compressed size for GOG Galaxy file part " << file.destination
						            << ": " << compressed_size << " != " << data.file.size;
					}
				} catch(...) {
					log_warning << "Could not parse compressed size for GOG Galaxy file part " << file.destination
					            << ": " << part_info[1];
				}
				
				try {
					
					// GOG Galaxy file parts are deflated, inflate them while extracting
					data.uncompressed_size = boost::lexical_cast<boost::uint64_t>(part_info[2]);
					data.file.filter = stream::ZlibFilter;
					
					file_start->size += data.uncompressed_size;
					
					if(&file != file_start) {
						
						// Ignore this file entry and instead add the data location to the start file
						file.destination.clear();
						file_start->additional_locations.push_back(file.location);
						
						if(file.components != file_start->components || file.tasks != file_start->tasks
						   || file.languages != file_start->languages || file.check != file_start->check
						   || file.options != file_start->options) {
							log_warning << "Mismatched constraints for different parts of GOG Galaxy file "
							            << file_start->destination << ": " << file.destination;
						}
						
					}
					
				} catch(...) {
					log_warning << "Could not parse size for GOG Galaxy file part " << file.destination
					            << ": " << part_info[1];
					remaining_parts = 0;
				}
				
			}
		} else if(!start_info.empty()) {
			log_warning << "Missing part info for GOG Galaxy file " << file.destination;
			remaining_parts = 0;
		} else if(remaining_parts != 0) {
			log_warning << "Incomplete GOG Galaxy file " << file_start->destination;
			remaining_parts = 0;
		}
		
	}
	
	if(remaining_parts != 0) {
		log_warning << "Incomplete GOG Galaxy file " << file_start->destination;
	}
	
}

} //namespace gog
