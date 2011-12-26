/*
 * Copyright (C) 2011 Daniel Scharrer
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
