
#ifndef INNOEXTRACT_SETUP_PERMISSION_HPP
#define INNOEXTRACT_SETUP_PERMISSION_HPP

#include <string>
#include <iosfwd>

namespace setup {

struct version;

struct permission_entry {
	
	// introduced in 4.1.0
	
	std::string permissions;
	
	void load(std::istream & is, const version & version);
	
};

} // namespace setup

#endif // INNOEXTRACT_SETUP_PERMISSION_HPP
