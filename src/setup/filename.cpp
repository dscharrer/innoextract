/*
 * Copyright (C) 2012-2020 Daniel Scharrer
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

#include <stddef.h>
#include <algorithm>
#include <cctype>

namespace setup {

namespace {

//! Check for separators in input paths.
struct is_path_separator {
	bool operator()(char c) {
		return (c == '\\' || c == '/');
	}
};

struct is_unsafe_path_char {
	bool operator()(char c) {
		if(static_cast<unsigned char>(c) < 32) {
			return true;
		}
		switch(c) {
			case '<': return true;
			case '>': return true;
			case ':': return true;
			case '"': return true;
			case '|': return true;
			case '?': return true;
			case '*': return true;
			default:  return false;
		}
	}
};

std::string replace_unsafe_chars(const std::string & str) {
	std::string result;
	result.resize(str.size());
	std::replace_copy_if(str.begin(), str.end(), result.begin(), is_unsafe_path_char(), '$');
	return result;
}

} // anonymous namespace

std::string filename_map::lookup(const std::string & key) const {
	std::map<std::string, std::string>::const_iterator i = find(key);
	return (i == end()) ? replace_unsafe_chars(key) : i->second;
}

std::string filename_map::expand_variables(it & begin, it end, bool close) const {
	
	std::string result;
	result.reserve(size_t(end - begin));
	
	while(begin != end) {
		
		// Flush everything before the next bracket
		it pos = begin;
		while(pos != end && *pos != '{' && *pos != '}') {
			++pos;
		}
		ptrdiff_t obegin = ptrdiff_t(result.size());
		result.append(begin, pos);
		std::replace_copy_if(result.begin() + obegin, result.end(), result.begin() + obegin,
		                     is_unsafe_path_char(), '$');
		begin = pos;
		
		if(pos == end) {
			// No more variables or escape sequences
			break;
		}
		
		begin++;
		
		if(close && *pos == '}') {
			// Current context closed
			break;
		}
		
		if(!close && *pos == '}') {
			// literal '}' character
			result.push_back('}');
			continue;
		}
		
		// '{{' escape sequence
		if(begin != end && *begin == '{') {
			result.push_back('{');
			begin++;
			continue;
		}
		
		// Recursively expand variables until we reach the end of this context
		result.append(lookup(expand_variables(begin, end, true)));
	}
	
	return result;
}

std::string filename_map::shorten_path(const std::string & path) {
	
	std::string result;
	result.reserve(path.size());
	
	it begin = path.begin();
	it end = path.end();
	while(begin != end) {
		
		it s_begin = begin;
		it s_end = std::find_if(begin, end, is_path_separator());
		begin = (s_end == end) ? end : (s_end + 1);
		
		size_t segment_length = size_t(s_end - s_begin);
		
		// Empty segment - ignore
		if(segment_length == 0) {
			continue;
		}
		
		// '.' segment - ignore
		if(segment_length == 1 && *s_begin == '.') {
			continue;
		}
		
		// '..' segment - backtrace in result path
		if(segment_length == 2 && *s_begin == '.' && *(s_begin + 1) == '.') {
			size_t last_sep = result.find_last_of(path_sep);
			if(last_sep == std::string::npos) {
				last_sep = 0;
			}
			result.resize(last_sep);
			continue;
		}
		
		// Normal segment - append to the result path
		if(!result.empty()) {
			result.push_back(path_sep);
		}
		result.append(s_begin, s_end);
		
	}
	
	return result;
}

std::string filename_map::convert(std::string path) const {
	
	// Convert paths to lower-case if requested
	if(lowercase) {
		std::transform(path.begin(), path.end(), path.begin(), ::tolower);
	}
	
	// Don't expand variables if requested
	if(!expand) {
		return path;
	}
	
	it begin = path.begin();
	std::string expanded = expand_variables(begin, path.end());
	
	return shorten_path(expanded);
}

} // namespace setup
