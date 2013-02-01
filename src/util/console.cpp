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

#include "util/console.hpp"

#include <algorithm>
#include <cmath>
#include <signal.h>
#include <iostream>
#include <cstdlib>

#include "configure.hpp"

#if defined(_WIN32)
#include <windows.h>
#endif

#if INNOEXTRACT_HAVE_ISATTY
#include <unistd.h>
#endif

#if INNOEXTRACT_HAVE_IOCTL
#include <sys/ioctl.h>
#endif

#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/lexical_cast.hpp>

#include "util/output.hpp"

static bool show_progress = true;

#if defined(SIGWINCH)

// The last known screen width.
static int screen_width;

// A flag that signals that the console may have been resized
static volatile sig_atomic_t screen_resized;

static void sigwinch_handler(int sig) {
	(void)sig;
	screen_resized = 1;
	signal(SIGWINCH, sigwinch_handler);
}

#endif

#if defined(_WIN32)
static HANDLE console_handle;
#endif

namespace color {

#if defined(_WIN32)

std::ostream & operator<<(std::ostream & os, const shell_command command) {
	
	color::current = command;
	
	if(command.command == boost::uint16_t(-1) || !console_handle) {
		// Color output is disabled
		return os;
	}
	
	if(&os != &std::cout && &os != &std::cerr) {
		// Colors are only supported for standard output
		return os;
	}
	
	std::cout.flush();
	std::cerr.flush();
	
	SetConsoleTextAttribute(console_handle, command.command);
	
	return os;
}

shell_command black =       { FOREGROUND_INTENSITY };
shell_command red =         { FOREGROUND_INTENSITY | FOREGROUND_RED };
shell_command green =       { FOREGROUND_INTENSITY | FOREGROUND_GREEN };
shell_command yellow =      { FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN };
shell_command blue =        { FOREGROUND_INTENSITY | FOREGROUND_BLUE };
shell_command magenta =     { FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_BLUE };
shell_command cyan =        { FOREGROUND_INTENSITY | FOREGROUND_BLUE | FOREGROUND_GREEN };
shell_command white =       { FOREGROUND_INTENSITY
                              | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE };

shell_command dim_black =   { 0 };
shell_command dim_red =     { FOREGROUND_RED };
shell_command dim_green =   { FOREGROUND_GREEN };
shell_command dim_yellow =  { FOREGROUND_RED | FOREGROUND_GREEN };
shell_command dim_blue =    { FOREGROUND_BLUE };
shell_command dim_magenta = { FOREGROUND_RED | FOREGROUND_BLUE };
shell_command dim_cyan =    { FOREGROUND_BLUE | FOREGROUND_GREEN };
shell_command dim_white =   { FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE };

shell_command reset =       dim_white;

struct resetter {
	shell_command original_color;
	~resetter() {
		std::cout << original_color;
	}
} resetter_instance = { white };

#else

shell_command black =       { "\x1b[1;30m" };
shell_command red =         { "\x1b[1;31m" };
shell_command green =       { "\x1b[1;32m" };
shell_command yellow =      { "\x1b[1;33m" };
shell_command blue =        { "\x1b[1;34m" };
shell_command magenta =     { "\x1b[1;35m" };
shell_command cyan =        { "\x1b[1;36m" };
shell_command white =       { "\x1b[1;37m" };

shell_command dim_black =   { "\x1b[0;30m" };
shell_command dim_red =     { "\x1b[0;31m" };
shell_command dim_green =   { "\x1b[0;32m" };
shell_command dim_yellow =  { "\x1b[0;33m" };
shell_command dim_blue =    { "\x1b[0;34m" };
shell_command dim_magenta = { "\x1b[0;35m" };
shell_command dim_cyan =    { "\x1b[0;36m" };
shell_command dim_white =   { "\x1b[0;37m" };

shell_command reset =       { "\x1b[0m" };

#endif

shell_command current = reset;

void init(is_enabled color, is_enabled progress) {
	
	bool is_tty = false;
#if INNOEXTRACT_HAVE_ISATTY
	is_tty = isatty(1) && isatty(2);
#endif
	
#if defined(_WIN32)
	console_handle = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_SCREEN_BUFFER_INFO info;
	if(console_handle && GetConsoleScreenBufferInfo(console_handle, &info)) {
		resetter_instance.original_color.command = info.wAttributes;
	} else {
		is_tty = false;
		color = disable;
		progress = disable;
	}
#endif
	
	// Initialize the progress bar
	
#if defined(SIGWINCH)
	sigwinch_handler(0);
#endif
	
	show_progress = (progress == enable);
#if defined(_WIN32) || (INNOEXTRACT_HAVE_IOCTL && defined(TIOCGWINSZ))
	// Only automatically enable the progress bar if we have a way to determine the width
	if(progress == automatic && is_tty) {
		show_progress = true;
	}
#endif
	
	// Initialize color output
	
	if(color == disable || (color == automatic && !is_tty)) {
		
#if defined(_WIN32)
		reset.command = boost::uint16_t(-1);
#else
		reset.command = "";
#endif
		
		black = red = green = yellow = blue = magenta = cyan = white = reset;
		dim_black = dim_red = dim_green = dim_yellow = reset;
		dim_blue = dim_magenta = dim_cyan = dim_white = reset;
		current = reset;
		
	}
	
#if defined(_WIN32)
	std::cout << reset;
#endif
	
}

} // namespace color

static int query_screen_width() {
	
#if defined(_WIN32)
	
	CONSOLE_SCREEN_BUFFER_INFO info;
	if(GetConsoleScreenBufferInfo(console_handle, &info)) {
		return info.srWindow.Right - info.srWindow.Left + 1;
	}
	
#endif

#if INNOEXTRACT_HAVE_IOCTL && defined(TIOCGWINSZ)
	
	struct winsize w;
	if(ioctl(0, TIOCGWINSZ, &w) >= 0) {
		return w.ws_col;
	}
	
#endif
	
	try {
		char * columns = std::getenv("COLUMNS");
		if(columns) {
			return boost::lexical_cast<int>(columns);
		}
	} catch(...) { /* ignore bad values */ }
	
	// Assume a default screen width of 80 columns
	return 80;
	
}

static int get_screen_width() {
	
#if defined(SIGWINCH)
	
	if(screen_resized) {
		screen_resized = 0;
		screen_width = query_screen_width();
	}
	
	return screen_width;
	
#else
	return query_screen_width();
#endif
	
}

static bool progress_cleared = true;

int progress::clear() {
	
	int width = get_screen_width();
	
	if(!show_progress) {
		return width;
	}
	
#if defined(_WIN32)
	
	// Overwrite the current line with whitespace
	
	static std::string buffer;
	static int last_width = 0;
	if(width != last_width) {
		size_t cwidth = size_t(std::max(width, 1) - 1);
		buffer.resize(cwidth, ' ');
		last_width = width;
	}
	
	std::cout << '\r' << buffer << '\r';
	
#else
	
	// Use the ANSI/VT100 control sequence to clear the current line
	
	std::cout << "\33[2K\r";
	
#endif
	
	progress_cleared = true;
	
	return width;
}

void progress::show(float value, const std::string & label) {
	
	if(!show_progress) {
		return;
	}
	
	int width = clear();
	
	std::ios_base::fmtflags flags = std::cout.flags();
	
	int progress_length = width - int(label.length()) - 6 - 2 - 2 - 1;
	
	if(progress_length > 10) {
		
		size_t progress = size_t(std::ceil(float(progress_length) * value));
		
		std::cout << '[';
		for(size_t i = 0; i < progress; i++) {
			std::cout << '=';
		}
		std::cout << '>';
		for(size_t i = progress; i < size_t(progress_length); i++) {
			std::cout << ' ';
		}
		std::cout << ']';
		
	}
	
	std::cout << std::right << std::fixed << std::setprecision(1) << std::setfill(' ')
	          << std::setw(5) << (value * 100) << "% " << label;
	std::cout.flush();
	
	std::cout.flags(flags);
	
	progress_cleared = false;
}

void progress::show_unbounded(float value, const std::string & label) {
	
	if(!show_progress) {
		return;
	}
	
	int width = clear();
	
	std::ios_base::fmtflags flags = std::cout.flags();
	
	int progress_length = width - int(label.length()) - 2 - 2 - 6;
	
	if(progress_length > 10) {
		
		size_t progress = std::min(size_t(std::ceil(float(progress_length) * value)),
		                  size_t(progress_length - 1));
		
		std::cout << '[';
		for(size_t i = 0; i < progress; i++) {
			std::cout << ' ';
		}
		std::cout << "<===>";
		for(size_t i = progress; i < size_t(progress_length); i++) {
			std::cout << ' ';
		}
		std::cout << ']';
		
	}
	
	std::cout << ' ' << label;
	std::cout.flush();
	
	std::cout.flags(flags);
	
	progress_cleared = false;
}

progress::progress(uint64_t max, bool show_rate)
	: max(max), value(0), show_rate(show_rate),
	  start_time(boost::posix_time::microsec_clock::universal_time()),
	  last_status(-1.f), last_time(0), last_rate(0.f) { }

void progress::update(uint64_t delta, bool force) {
	
	if(!show_progress) {
		return;
	}
	
	force = force || progress_cleared;
	
	value += delta;
	
	float status = 0.f;
	if(max) {
		status = float(std::min(value, max)) / float(max);
		status = float(size_t(1000.f * status)) * (1.f / 1000.f);
		if(!force && status == last_status) {
			return;
		}
	}
	
	boost::posix_time::ptime now(boost::posix_time::microsec_clock::universal_time());
	uint64_t time = uint64_t((now - start_time).total_microseconds());
	
	const uint64_t update_interval = 100000;
	if(!force && time - last_time < update_interval) {
		return;
	}
	
	last_time = time;
	last_status = status;
	
	if(!max) {
		status = std::fmod(float(time) * (1.f / 5000000.f), 2.f);
		if(status > 1.f) {
			status = 2.f - status;
		}
	}
	
	if(show_rate) {
		if(value >= 10 * 1024 && time > 0) {
			float rate = 1000000.f * float(value) / float(time);
			if(rate != last_rate) {
				last_rate = rate;
				label.str(std::string()); // clear the buffer
				label << std::right << std::fixed << std::setfill(' ') << std::setw(5)
				      << print_bytes(rate, 1) << "/s";
			}
		}
	}
	
	if(max) {
		show(status, label.str());
	} else {
		show_unbounded(status, label.str());
	}
	
}

void progress::set_enabled(bool enable) {
	show_progress = enable;
}
