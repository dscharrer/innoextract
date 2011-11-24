
#ifndef INNOEXTRACT_UTIL_CONSOLE_HPP
#define INNOEXTRACT_UTIL_CONSOLE_HPP

#include <ostream>
#include <iomanip>

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

} // namespace color

inline std::ostream & operator<<(std::ostream & os, const color::shell_command command) {
	color::current = command;
	return os << command.command;
}

namespace progress {

void show(float value, const std::string & label);

void clear();

} // namespace

#endif // INNOEXTRACT_UTIL_CONSOLE_HPP
