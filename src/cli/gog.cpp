/*
 * Copyright (C) 2014 Daniel Scharrer
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

#include "cli/gog.hpp"

#include <stddef.h>
#include <cstring>

#include <boost/foreach.hpp>
#include <boost/algorithm/string/predicate.hpp>

#include "setup/info.hpp"
#include "setup/registry.hpp"

namespace gog {

std::string get_game_id(const setup::info & info) {
	
	std::string id;
	
	const char * prefix = "SOFTWARE\\GOG.com\\Games\\";
	size_t prefix_length = std::strlen(prefix);
	
	BOOST_FOREACH(const setup::registry_entry & entry, info.registry_entries) {
		
		if(!boost::istarts_with(entry.key, prefix)) {
			continue;
		}
		
		if(entry.key.find('\\', prefix_length) != std::string::npos) {
			continue;
		}
		
		if(boost::iequals(entry.name, "gameID")) {
			return entry.value;
		}
		
		if(id.empty()) {
			id = entry.key.substr(prefix_length);
		}
		
	}
	
	return id;
}

} // namespace gog
