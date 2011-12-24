
#ifndef INNOEXTRACT_UTIL_CONSOLE_HPP
#define INNOEXTRACT_UTIL_CONSOLE_HPP

#include <stddef.h>
#include <stdint.h>
#include <ostream>
#include <iomanip>
#include <sstream>

#include <boost/date_time/posix_time/ptime.hpp>

namespace color {

struct shell_command {
	const char * command;
};

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

void init();

inline std::ostream & operator<<(std::ostream & os, const shell_command command) {
	color::current = command;
	return os << command.command;
}

} // namespace color

class progress {
	
	uint64_t max;
	uint64_t value;
	bool show_rate;
	
	boost::posix_time::ptime start_time;
	
	float last_status;
	uint64_t last_time;
	
	float last_rate;
	std::ostringstream label;
	
public:
	
	progress(uint64_t max = 0, bool show_rate = true);
	progress(const progress & o)
		: max(o.max), value(o.value), show_rate(o.show_rate), start_time(o.start_time),
		  last_status(o.last_status), last_time(o.last_time),
		  last_rate(o.last_rate), label(o.label.str()) { }
	
	void update(uint64_t delta = 0, bool force = false);
	
	static void show(float value, const std::string & label = std::string());
	
	static void show_unbounded(float value, const std::string & label = std::string());
	
	static void clear();
	
};

#endif // INNOEXTRACT_UTIL_CONSOLE_HPP
