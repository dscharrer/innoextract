/*
 * Copyright (C) 2011-2019 Daniel Scharrer
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

#include "setup/message.hpp"

#include <boost/cstdint.hpp>

#include "setup/info.hpp"
#include "setup/language.hpp"
#include "setup/version.hpp"
#include "util/encoding.hpp"
#include "util/load.hpp"
#include "util/log.hpp"

namespace setup {

void message_entry::load(std::istream & is, const info & i) {
	
	is >> util::encoded_string(name, i.codepage);
	is >> util::binary_string(value);
	
	language = util::load<boost::int32_t>(is);
	
	boost::uint32_t codepage;
	if(language < 0) {
		codepage = i.codepage;
	} else if(size_t(language) >= i.languages.size()) {
		if(!i.languages.empty()) {
			log_warning << "Language index out of bounds: " << language;
		}
		value.clear();
		return;
	} else {
		codepage = i.languages[size_t(language)].codepage;
	}
	
	util::to_utf8(value, codepage);
}

} // namespace setup
