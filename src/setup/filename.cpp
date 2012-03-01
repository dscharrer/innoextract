/*
 * Copyright (C) 2012 Daniel Scharrer
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

#include "setup/filename.hpp"

#include <algorithm>
#include <cctype>

#include "util/log.hpp"

using std::string;
namespace fs = boost::filesystem;

namespace setup {

const string & filename_map::lookup(const string & key) const {
	std::map<string, string>::const_iterator i = find(key);
	return (i == end()) ? key : i->second;
}

fs::path filename_map::convert(const string & name) const {
	
	size_t start = 0;
	string buffer;
	fs::path result;
	
	while(true) {
		
		size_t pos = name.find_first_of("{\\", start);
		
		if(pos == string::npos || name[pos] == '\\') {
			
			// Directory segment without constant
			
			size_t n = (pos == string::npos) ? string::npos : pos - start;
			string segment = name.substr(start, n);
			
			if(lowercase) {
				std::transform(segment.begin(), segment.end(), segment.begin(), ::tolower);
			}
			
			if(buffer.empty()) {
				result /= segment;
			} else {
				result /= buffer + segment;
				buffer.clear();
			}
			
			if(pos == string::npos) {
				return result;
			}
			
			start = pos + 1;
			
		} else {
			
			// Constant or escape sequence
			
			string segment = name.substr(start, pos - start);
			if(lowercase) {
				std::transform(segment.begin(), segment.end(), segment.begin(), ::tolower);
			}
			buffer += segment;
			
			if(pos + 1 < name.length() && name[pos + 1] == '{') {
				
				// Handle '{{' escape sequence
				buffer += '{';
				start = pos + 2;
				
			} else {
				
				// Handle nested constants.
				size_t count = 1;
				size_t end = pos;
				do {
					end = name.find_first_of("}{", end + 1);
					(name[end] == '}') ? count-- : count++;
				} while(count > 0 && end != string::npos);
				
				if(end == string::npos) {
					start = pos + 1;
				} else {
					string key = name.substr(pos + 1, end - pos - 1);
					std::transform(key.begin(), key.end(), key.begin(), ::tolower);
					buffer += lookup(key);
					start = end + 1;
				}
				
			}
		}
		
	}
}

} // namespace setup
