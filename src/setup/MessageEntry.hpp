
#ifndef INNOEXTRACT_SETUP_CUSTOMMESSAGEENTRY_HPP
#define INNOEXTRACT_SETUP_CUSTOMMESSAGEENTRY_HPP

#include <string>
#include <iosfwd>

#include "setup/version.hpp"

struct MessageEntry {
	
	// introduced in 4.2.1
	
	// UTF-8 encoded name.
	std::string name;
	
	// Value encoded in the codepage specified at language index.
	std::string value;
	
	// Index into the default language entry list or -1.
	int language;
	
	void load(std::istream & is, const inno_version & version);
	
};

#endif // INNOEXTRACT_SETUP_CUSTOMMESSAGEENTRY_HPP
