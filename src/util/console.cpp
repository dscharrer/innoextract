/*
 * Copyright (C) 2011-2019 Daniel Scharrer
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
#include <cstdio>
#include <cstring>

#include "configure.hpp"

#if INNOEXTRACT_HAVE_ISATTY
#include <unistd.h>
#endif

#if INNOEXTRACT_HAVE_IOCTL
#include <sys/ioctl.h>
#endif

#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/foreach.hpp>

#include "util/output.hpp"
#include "util/windows.hpp"

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

namespace color {

shell_command black =       { "\x1b[1;30m" };
shell_command red =         { "\x1b[1;31m" };
shell_command green =       { "\x1b[1;32m" };
shell_command yellow =      { "\x1b[1;33m" };
shell_command blue =        { "\x1b[1;34m" };
shell_command magenta =     { "\x1b[1;35m" };
shell_command cyan =        { "\x1b[1;36m" };
shell_command white =       { "\x1b[1;37m" };

shell_command dim_black =   { "\x1b[;30m" };
shell_command dim_red =     { "\x1b[;31m" };
shell_command dim_green =   { "\x1b[;32m" };
shell_command dim_yellow =  { "\x1b[;33m" };
shell_command dim_blue =    { "\x1b[;34m" };
shell_command dim_magenta = { "\x1b[;35m" };
shell_command dim_cyan =    { "\x1b[;36m" };
shell_command dim_white =   { "\x1b[;37m" };

shell_command reset =       { "\x1b[m" };

shell_command current = reset;

void init(is_enabled color, is_enabled progress) {
	
	bool is_tty;
	#if defined(_WIN32) || INNOEXTRACT_HAVE_ISATTY
	is_tty = isatty(1) && isatty(2);
	#else
	// Since we can't check if stdout is a terminal,
	// don't automatically enable color output and progress bar
	is_tty = false;
	#endif
	
	#if !defined(_WIN32)
	if(is_tty) {
		char * term = std::getenv("TERM");
		if(!term || !std::strcmp(term, "dumb")) {
			is_tty = false; // Terminal does not support escape sequences
		}
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
	
	if(color == disable || (color == automatic && (!is_tty || std::getenv("NO_COLOR") != NULL))) {
		
		reset.command = "";
		current.command = "";
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
		
	} else {
		
		#if defined(_WIN32)
		// For our Windows abstraction, the default color might differ from the initial one
		std::cout << reset;
		std::cerr << reset;
		#endif
		
	}
	
}

} // namespace color

static int query_screen_width() {
	
	#if defined(_WIN32)
	
	int width = util::console_width();
	if(width) {
		return width;
	}
	
	#endif
	
	#if INNOEXTRACT_HAVE_IOCTL && defined(TIOCGWINSZ)
	
	struct winsize w;
	if(ioctl(0, TIOCGWINSZ, &w) >= 0) {
		return w.ws_col;
	}
	
	#endif
	
	#if !defined(_WIN32)
	try {
		char * columns = std::getenv("COLUMNS");
		if(columns) {
			return boost::lexical_cast<int>(columns);
		}
	} catch(...) { /* ignore bad values */ }
	#endif
	
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

void progress::clear(ClearMode mode) {
	
	if(!show_progress) {
		return;
	}
	
	progress_cleared = true;
	
	#if defined(_WIN32)
	
	if(mode == FastClear) {
		
		/*
		 * If we overwrite the whole line with spaces, windows console likes to draw
		 * the empty line, even if it will be overwritten in the same flush(),
		 * causing the progress bar to flicker when updated.
		 * To work around this, don't actually clear the line if we are just going to
		 * overwrite it anyway.
		 * The progress bar still flickers when there is other output printed, but
		 * it seems there is no way around that without using the console API to manually
		 * scroll the output.
		 */
		
		std::cout << '\r';
		
		return;
		
	} else if(mode == DeferredClear && isatty(1)) {
		
		/*
		 * Special clear mode that leaves the original line but insert new lines before it
		 * until the next carriage return.
		 */
		
		std::cout << "\r\x1b[3K";
		
		return;
		
	}
	
	#else
	
	(void)mode;
	
	#endif
	
	// Use the ANSI/VT100 control sequence to clear the current line
	
	std::cout << "\r\x1b[K";
	
}

void progress::show(float value, const std::string & label) {
	
	if(!show_progress) {
		return;
	}
	
	clear(FastClear);
	
	int width = get_screen_width();
	
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
	
	clear(FastClear);
	
	int width = get_screen_width();
	
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

progress::progress(boost::uint64_t max_value, bool show_value_rate)
	: max(max_value), value(0), show_rate(show_value_rate),
	  start_time(boost::posix_time::microsec_clock::universal_time()),
	  last_status(-1.f), last_time(0), last_rate(0.f) { }

bool progress::update(boost::uint64_t delta, bool force) {
	
	if(!show_progress) {
		return false;
	}
	
	force = force || progress_cleared;
	
	value += delta;
	
	float status = 0.f;
	if(max) {
		status = float(std::min(value, max)) / float(max);
		status = float(size_t(1000.f * status)) * (1.f / 1000.f);
		if(!force && status == last_status) {
			return false;
		}
	}
	
	boost::uint64_t time;
	try {
		boost::posix_time::ptime now(boost::posix_time::microsec_clock::universal_time());
		time = boost::uint64_t((now - start_time).total_microseconds());
	} catch(...) {
		// this shouldn't happen, assume no time has passed
		time = last_time;
	}
	
	#if defined(_WIN32)
	const boost::uint64_t update_interval = 100000;
	#else
	const boost::uint64_t update_interval = 50000;
	#endif
	if(!force && time - last_time < update_interval) {
		return false;
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
	
	return true;
}

void progress::set_enabled(bool enable) {
	show_progress = enable;
}

bool progress::is_enabled() {
	return show_progress;
}

