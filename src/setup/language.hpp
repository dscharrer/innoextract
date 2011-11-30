
#ifndef INNOEXTRACT_SETUP_LANGUAGE_HPP
#define INNOEXTRACT_SETUP_LANGUAGE_HPP

#include <stdint.h>
#include <string>
#include <iosfwd>

namespace setup {

struct version;

struct language_entry {
	
	// introduced in 2.0.1
	
	std::string name;
	std::string language_name;
	std::string dialog_font;
	std::string title_font;
	std::string welcome_font;
	std::string copyright_font;
	std::string data;
	std::string license_text;
	std::string info_before;
	std::string info_after;
	
	uint32_t language_id;
	uint32_t codepage;
	size_t dialog_font_size;
	size_t dialog_font_standard_height;
	size_t title_font_size;
	size_t welcome_font_size;
	size_t copyright_font_size;
	
	bool right_to_left;
	
	void load(std::istream & is, const version & version);
	
};

} // namespace setup

#endif // INNOEXTRACT_SETUP_LANGUAGE_HPP
