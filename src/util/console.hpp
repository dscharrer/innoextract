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

#ifndef INNOEXTRACT_UTIL_CONSOLE_HPP
#define INNOEXTRACT_UTIL_CONSOLE_HPP

#include <stddef.h>
#include <ostream>
#include <iomanip>
#include <sstream>

#include <boost/date_time/posix_time/ptime.hpp>
#include <boost/cstdint.hpp>

namespace color {


struct shell_command {
#if defined(_WIN32)
	boost::uint16_t command;
#else
	const char * command;
#endif
};

std::ostream & operator<<(std::ostream & os, const shell_command command);

extern shell_command reset;

extern shell_command black;
extern shell_command red;
extern shell_command green;
extern shell_command yellow;
extern shell_command blue;
extern shell_command magenta;
extern shell_command cyan;
extern shell_command white;

extern shell_command dim_black;
extern shell_command dim_red;
extern shell_command dim_green;
extern shell_command dim_yellow;
extern shell_command dim_blue;
extern shell_command dim_magenta;
extern shell_command dim_cyan;
extern shell_command dim_white;

extern shell_command current;

#if !defined(_WIN32)
inline std::ostream & operator<<(std::ostream & os, const shell_command command) {
	color::current = command;
	return os << command.command;
}
#endif

enum is_enabled {
	enable,
	disable,
	automatic
};

void init(is_enabled color = automatic, is_enabled progress = automatic);

} // namespace color

class progress {
	
	boost::uint64_t max;
	boost::uint64_t value;
	bool show_rate;
	
	boost::posix_time::ptime start_time;
	
	float last_status;
	boost::uint64_t last_time;
	
	float last_rate;
	std::ostringstream label;
	
public:
	
	progress(boost::uint64_t max = 0, bool show_rate = true);
	progress(const progress & o)
		: max(o.max), value(o.value), show_rate(o.show_rate), start_time(o.start_time),
		  last_status(o.last_status), last_time(o.last_time),
		  last_rate(o.last_rate), label(o.label.str()) { }
	
	void update(boost::uint64_t delta = 0, bool force = false);
	
	static void show(float value, const std::string & label = std::string());
	
	static void show_unbounded(float value, const std::string & label = std::string());
	
	static int clear();
	
	static void set_enabled(bool enable);
	
};

#endif // INNOEXTRACT_UTIL_CONSOLE_HPP
