
#ifndef INNOEXTRACT_CUSTOMMESSAGEENTRY_HPP
#define INNOEXTRACT_CUSTOMMESSAGEENTRY_HPP

#include <iostream>
#include "Version.hpp"

struct CustomMessageEntry {
	
	// introduced in 4.2.1
	
	// UTF-8 encoded name.
	std::string name;
	
	// Value encoding in the codepage specified at language index.
	std::string value;
	
	// Index into the default language entry list or -1.
	int language;
	
	void load(std::istream & is, const InnoVersion & version);
	
};

#endif // INNOEXTRACT_CUSTOMMESSAGEENTRY_HPP
