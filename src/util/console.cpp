
#include "util/console.hpp"

#include <iostream>
#include <cmath>

#include "configure.hpp"

#ifdef HAVE_ISATTY
#include <unistd.h>
#endif

#ifdef HAVE_IOCTL
#include <sys/ioctl.h>
#endif

static bool native_console = true;

namespace color {

shell_command reset = { "\x1b[0m" };

shell_command black = { "\x1b[1;30m" };
shell_command red = { "\x1b[1;31m" };
shell_command green = { "\x1b[1;32m" };
shell_command yellow = { "\x1b[1;33m" };
shell_command blue = { "\x1b[1;34m" };
shell_command magenta = { "\x1b[1;35m" };
shell_command cyan = { "\x1b[1;36m" };
shell_command white = { "\x1b[1;37m" };

shell_command dim_black = { "\x1b[0;30m" };
shell_command dim_red = { "\x1b[0;31m" };
shell_command dim_green = { "\x1b[0;32m" };
shell_command dim_yellow = { "\x1b[0;33m" };
shell_command dim_blue = { "\x1b[0;34m" };
shell_command dim_magenta = { "\x1b[0;35m" };
shell_command dim_cyan = { "\x1b[0;36m" };
shell_command dim_white = { "\x1b[0;37m" };

shell_command current = reset;

void init() {
	
#ifdef HAVE_ISATTY
	if(isatty(1) && isatty(2)) {
		return;
	}
#endif
	
	native_console = false;
	
	reset.command = "";
	
	black.command = "";
	red.command = "";
	green.command = "";
	yellow.command = "";
	blue.command = "";
	magenta.command = "";
	cyan.command = "";
	white.command = "";
	
	dim_black.command = "";
	dim_red.command = "";
	dim_green.command = "";
	dim_yellow.command = "";
	dim_blue.command = "";
	dim_magenta.command = "";
	dim_cyan.command = "";
	dim_white.command = "";
	
	current = reset;
	
}

} // namespace color

namespace progress {

void show(float value, const std::string & label) {
	
	if(!native_console) {
		return;
	}
	
#if defined(HAVE_IOCTL) && defined(TIOCGWINSZ)
	
	struct winsize w;
	ioctl(0, TIOCGWINSZ, &w);
	
	clear();
	
	std::ios_base::fmtflags flags = std::cout.flags();
	
	size_t progress_length = w.ws_col - label.length() - 6 - 2 - 2 - 1;
	
	if(progress_length > 10) {
		
		size_t progress = size_t(std::ceil(float(progress_length) * value));
		
		std::cout << '[';
		for(size_t i = 0; i < progress; i++) {
			std::cout << '=';
		}
		std::cout << '>';
		for(size_t i = progress; i < progress_length; i++) {
			std::cout << ' ';
		}
		std::cout << ']';
		
	}
	
	std::cout << std::right << std::fixed << std::setprecision(1) << std::setfill(' ')
						<< std::setw(5) << (value * 100) << "% " << label;
	std::cout.flush();
	
	std::cout.flags(flags);
	
#endif
	
}

void clear() {
	
	if(!native_console) {
		return;
	}
	
#ifdef HAVE_IOCTL
	
	std::cout << "\33[2K\r";
	
#endif
	
}

} // namespace progress
