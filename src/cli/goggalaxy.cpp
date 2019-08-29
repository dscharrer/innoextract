/*
 * Copyright (C) 2018-2019 Daniel Scharrer
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

#include <set>
#include <string>
#include <vector>

#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/trim.hpp>

#include "crypto/checksum.hpp"

#include "setup/data.hpp"
#include "setup/file.hpp"
#include "setup/info.hpp"
#include "setup/language.hpp"

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

int parse_hex(char c) {
	if(c >= '0' && c <= '9') {
		return c - '0';
	} else if(c >= 'a' && c <= 'f') {
		return c - 'a' + 10;
	} else if(c >= 'A' && c <= 'F') {
		return c - 'a' + 10;
	} else {
		return -1;
	}
}

crypto::checksum parse_checksum(const std::string & string) {
	
	crypto::checksum checksum;
	checksum.type = crypto::MD5;
	
	if(string.length() != 32) {
		// Unknown checksum type
		checksum.type = crypto::None;
		return checksum;
	}
	
	for(size_t i = 0; i < 16; i++) {
		int a = parse_hex(string[2 * i]);
		int b = parse_hex(string[2 * i + 1]);
		if(a < 0 || b < 0) {
			checksum.type = crypto::None;
			break;
		}
		checksum.md5[i] = char((a << 4) | b);
	}
	
	return checksum;
}

struct constraint {
	
	std::string name;
	bool negated;
	
	explicit constraint(const std::string & constraint_name, bool is_negated = false)
		: name(constraint_name), negated(is_negated) { }
	
};

std::vector<constraint> parse_constraints(const std::string & input) {
	
	std::vector<constraint> result;
	
	size_t start = 0;
	
	while(start < input.length()) {
		
		start = input.find_first_not_of(" \t\r\n", start);
		if(start == std::string::npos) {
			break;
		}
		
		bool negated = false;
		if(input[start] == '!') {
			negated = true;
			start++;
		}
		
		size_t end = input.find('#', start);
		if(end == std::string::npos) {
			end = input.length();
		}
		
		if(end != start) {
			std::string token = input.substr(start, end - start);
			boost::trim(token);
			result.push_back(constraint(token, negated));
		}
		
		if(end == std::string::npos) {
			end = input.length();
		}
		
		start = end + 1;
	}
	
	return result;
}

std::string create_constraint_expression(std::vector<constraint> & constraints) {
	
	std::string result;
	
	BOOST_FOREACH(const constraint & entry, constraints) {
		
		if(!result.empty()) {
			result += " or ";
		}
		
		if(entry.negated) {
			result += " not ";
		}
		
		result += entry.name;
		
	}
	
	return result;
}

} // anonymous namespace

void parse_galaxy_files(setup::info & info, bool force) {
	
	if(!force) {
		bool is_gog = boost::icontains(info.header.app_publisher, "GOG.com");
		is_gog = is_gog || boost::icontains(info.header.app_publisher_url, "www.gog.com");
		is_gog = is_gog || boost::icontains(info.header.app_support_url, "www.gog.com");
		is_gog = is_gog || boost::icontains(info.header.app_updates_url, "www.gog.com");
		if(!is_gog) {
			return;
		}
	}
	
	setup::file_entry * file_start = NULL;
	size_t remaining_parts = 0;
	
	bool has_language_constraints = false;
	std::set<std::string> all_languages;
	
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
			
			file.checksum = parse_checksum(start_info[0]);
			file.size = 0;
			if(file.checksum.type == crypto::None) {
				log_warning << "Could not parse checksum for GOG Galaxy file " << file.destination
				            << ": " << start_info[0];
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
		
		if(!file.destination.empty()) {
			// languages, architectures, winversions
			std::vector<std::string> check = parse_function_call(file.check, "check_if_install");
			if(!check.empty() && !check[0].empty()) {
				std::vector<constraint> languages = parse_constraints(check[0]);
				BOOST_FOREACH(const constraint & language, languages) {
					all_languages.insert(language.name);
				}
			}
		}
		
		has_language_constraints = has_language_constraints || !file.languages.empty();
		
	}
	
	if(remaining_parts != 0) {
		log_warning << "Incomplete GOG Galaxy file " << file_start->destination;
	}
	
	/*
	 * GOG Galaxy multi-part files also have their own constraints, convert these to standard
	 * Inno Setup ones.
	 *
	 * Do this in a separate loop to not break constraint checks above.
	 */
	
	BOOST_FOREACH(setup::file_entry & file, info.files) {
		
		if(file.destination.empty()) {
			continue;
		}
		
		// languages, architectures, winversions
		std::vector<std::string> check = parse_function_call(file.check, "check_if_install");
		if(!check.empty()) {
			
			if(!check[0].empty()) {
				
				std::vector<constraint> languages = parse_constraints(check[0]);
				
				// Ignore constraints that just contain all languages
				bool has_all_languages = false;
				if(languages.size() >= all_languages.size() && all_languages.size() > 1) {
					has_all_languages = true;
					BOOST_FOREACH(const std::string & known_language, all_languages) {
						bool has_language = false;
						BOOST_FOREACH(const constraint & language, languages) {
							if(!language.negated && language.name == known_language) {
								has_language = true;
								break;
							}
						}
						if(!has_language) {
							has_all_languages = false;
							break;
						}
					}
				}
				
				if(!languages.empty() && !has_all_languages) {
					if(!file.languages.empty()) {
						log_warning << "Overwriting language constraints for GOG Galaxy file " << file.destination;
					}
					file.languages = create_constraint_expression(languages);
				}
				
			}
			
			if(check.size() >= 2 && !check[1].empty()) {
				const setup::file_entry::flags all_arch = setup::file_entry::Bits32 | setup::file_entry::Bits64;
				setup::file_entry::flags arch = 0;
				if(check[1] != "32#64#") {
					std::vector<constraint> architectures = parse_constraints(check[1]);
					BOOST_FOREACH(const constraint & architecture, architectures) {
						if(architecture.negated && architectures.size() > 1) {
							log_warning << "Ignoring architecture for GOG Galaxy file " << file.destination
							            << ": !" << architecture.name;
						} else if(architecture.name == "32") {
							arch |= setup::file_entry::Bits32;
						} else if(architecture.name == "64") {
							arch |= setup::file_entry::Bits64;
						} else {
							log_warning << "Unknown architecture for GOG Galaxy file " << file.destination
							            << ": " << architecture.name;
						}
						if(architecture.negated && architectures.size() <= 1) {
							arch = all_arch & ~arch;
						}
					}
					if(arch == all_arch) {
						arch = 0;
					}
				}
				if((file.options & all_arch) && (file.options & all_arch) != arch) {
					log_warning << "Overwriting architecture constraints for GOG Galaxy file " << file.destination;
				}
				file.options = (file.options & ~all_arch) | arch;
			}
			
			if(check.size() >= 3 && !check[2].empty()) {
				log_warning << "Ignoring OS constraint for GOG Galaxy file " << file.destination
				            << ": " << check[2];
			}
			
			if(file.components.empty()) {
				file.components = "game";
			}
			
		}
		
		// component id, ?
		std::vector<std::string> dependency = parse_function_call(file.check, "check_if_install_dependency");
		if(!dependency.empty()) {
			if(file.components.empty() && !dependency[0].empty()) {
				file.components = dependency[0];
			}
		}
		
	}
	
	if(!all_languages.empty()) {
		if(!has_language_constraints) {
			info.languages.clear();
		}
		info.languages.reserve(all_languages.size());
		BOOST_FOREACH(const std::string & name, all_languages) {
			setup::language_entry language;
			language.name = name;
			language.dialog_font_size = 0;
			language.dialog_font_standard_height = 0;
			language.title_font_size = 0;
			language.welcome_font_size = 0;
			language.copyright_font_size = 0;
			language.right_to_left = false;
			info.languages.push_back(language);
		}
	}
	
}

} // namespace gog
