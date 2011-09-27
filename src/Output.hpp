
#ifndef INNOEXTRACT_COLOROUT_HPP
#define INNOEXTRACT_COLOROUT_HPP

#include <iostream>
#include <iomanip>

namespace color {
	
	struct shell_command {
		
		int c0;
		int c1;
		
	};
	
	const shell_command reset = { 0, 0 };
	
	const shell_command black = { 1, 30 };
	const shell_command red = { 1, 31 };
	const shell_command green = { 1, 32 };
	const shell_command yellow = { 1, 33 };
	const shell_command blue = { 1, 34 };
	const shell_command magenta = { 1, 35 };
	const shell_command cyan = { 1, 36 };
	const shell_command white = { 1, 37 };
	
	const shell_command dim_black = { 0, 30 };
	const shell_command dim_red = { 0, 31 };
	const shell_command dim_green = { 0, 32 };
	const shell_command dim_yellow = { 0, 33 };
	const shell_command dim_blue = { 0, 34 };
	const shell_command dim_magenta = { 0, 35 };
	const shell_command dim_cyan = { 0, 36 };
	const shell_command dim_white = { 0, 37 };
	
	extern shell_command current;
	
};

inline std::ostream & operator<<(std::ostream & os, const color::shell_command command) {
	color::current = command;
	std::ios_base::fmtflags old = os.flags();
	os << "\x1B[" << std::dec << command.c0 << ';' << command.c1 << 'm';
	os.setf(old, std::ios_base::basefield);
	return os;
}

struct error_base {
	
	color::shell_command previous;
	
	inline error_base(color::shell_command type) : previous(color::current) {
		std::cerr << type << "!! ";
	}
	
	inline ~error_base() {
		std::cerr << previous << std::endl;
	}
	
};

#define error (error_base(color::red), std::cerr)
#define warning (error_base(color::yellow), std::cerr)

#endif // INNOEXTRACT_COLOROUT_HPP
