
#ifndef INNOEXTRACT_SETUP_MESSAGE_HPP
#define INNOEXTRACT_SETUP_MESSAGE_HPP

#include <string>
#include <iosfwd>

namespace setup {

struct version;

struct message_entry {
	
	// introduced in 4.2.1
	
	// UTF-8 encoded name.
	std::string name;
	
	// Value encoded in the codepage specified at language index.
	std::string value;
	
	// Index into the default language entry list or -1.
	int language;
	
	void load(std::istream & is, const version & version);
	
};

} // namespace setup

#endif // INNOEXTRACT_SETUP_MESSAGE_HPP
