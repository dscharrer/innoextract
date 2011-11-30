
#ifndef INNOEXTRACT_SETUP_TYPE_HPP
#define INNOEXTRACT_SETUP_TYPE_HPP

#include <stdint.h>
#include <string>
#include <iosfwd>

#include "setup/windows.hpp"
#include "util/enum.hpp"
#include "util/flags.hpp"

namespace setup {

struct version;

struct type_entry {
	
	// introduced in 2.0.0
	
	enum setup_type {
		User,
		DefaultFull,
		DefaultCompact,
		DefaultCustom
	};
	
	std::string name;
	std::string description;
	std::string languages;
	std::string check;
	
	windows_version_range winver;
	
	bool custom_type;
	
	setup_type type;
	
	uint64_t size;
	
	void load(std::istream & is, const version & version);
	
};

} // namespace setup

NAMED_ENUM(setup::type_entry::setup_type)

#endif // INNOEXTRACT_SETUP_TYPE_HPP
