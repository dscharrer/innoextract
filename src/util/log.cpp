
#include "util/log.hpp"

#include <iostream>

#include "util/console.hpp"

bool logger::debug = false;

size_t logger::total_errors = 0;
size_t logger::total_warnings = 0;

logger::~logger() {
	
	color::shell_command previous = color::current;
	progress::clear();
	
	switch(level) {
		case Debug:   std::cout << color::cyan   << buffer.str() << std::endl; break;
		case Info:    std::cout << color::white  << buffer.str() << std::endl; break;
		case Warning: std::cerr << color::yellow << buffer.str() << std::endl; break;
		case Error:   std::cerr << color::red    << buffer.str() << std::endl; break;
	}
	
	std::cout << previous;
}
