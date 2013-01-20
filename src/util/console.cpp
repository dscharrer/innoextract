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

#include "configure.hpp"

#if INNOEXTRACT_HAVE_ISATTY
#include <unistd.h>
#endif

#if INNOEXTRACT_HAVE_IOCTL
#include <sys/ioctl.h>
#endif

#include <boost/date_time/posix_time/posix_time_types.hpp>

#include "util/output.hpp"

static bool show_progress = true;

#if INNOEXTRACT_HAVE_IOCTL && defined(TIOCGWINSZ) && defined(SIGWINCH)

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

void init(is_enabled color, is_enabled progress) {
	
#if INNOEXTRACT_HAVE_IOCTL && defined(TIOCGWINSZ) && defined(SIGWINCH)
	sigwinch_handler(0);
#endif
	
#if INNOEXTRACT_HAVE_ISATTY
	bool is_tty = isatty(1) && isatty(2);
#else
	bool is_tty = false;
#endif
	
	show_progress = (progress == enable) || (progress == automatic && is_tty);
	
	if(color == disable || (color == automatic && !is_tty)) {
		
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
}

} // namespace color

#if INNOEXTRACT_HAVE_IOCTL && defined(TIOCGWINSZ)

static int query_screen_width() {
	
	struct winsize w;
	if(ioctl(0, TIOCGWINSZ, &w) >= 0) {
		return w.ws_col;
	}
	
	// Assume a default screen width of 80 columns
	return 80;
}

static int get_screen_width() {
	
#ifdef SIGWINCH
	
	if(screen_resized) {
		screen_resized = 0;
		screen_width = query_screen_width();
	}
	
	return screen_width;
	
#else
	return query_screen_width();
#endif
	
}

#endif

void progress::show(float value, const std::string & label) {
	
	(void)value, (void)label;
	
	if(!show_progress) {
		return;
	}
	
#if INNOEXTRACT_HAVE_IOCTL && defined(TIOCGWINSZ)
	
	int width = get_screen_width();
	
	clear();
	
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
	
#endif
	
}

void progress::show_unbounded(float value, const std::string & label) {
	
	if(!show_progress) {
		return;
	}
	
#if INNOEXTRACT_HAVE_IOCTL && defined(TIOCGWINSZ)
	
	int width = get_screen_width();
	
	clear();
	
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
	
#else
	
	(void)value, (void)label;
	
#endif
	
}

#if INNOEXTRACT_HAVE_IOCTL && defined(TIOCGWINSZ)
static bool progress_cleared = true;
#endif

void progress::clear() {
	
	if(!show_progress) {
		return;
	}
	
#if INNOEXTRACT_HAVE_IOCTL && defined(TIOCGWINSZ)
	
	std::cout << "\33[2K\r";
	
	progress_cleared = true;
	
#endif
}

progress::progress(uint64_t max, bool show_rate)
	: max(max), value(0), show_rate(show_rate),
	  start_time(boost::posix_time::microsec_clock::universal_time()),
	  last_status(-1.f), last_time(0), last_rate(0.f) { }

void progress::update(uint64_t delta, bool force) {
	
	if(!show_progress) {
		return;
	}
	
#if INNOEXTRACT_HAVE_IOCTL && defined(TIOCGWINSZ)
	
	force = force || progress_cleared;
	progress_cleared = false;
	
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
	
	if(!force && time - last_time < 100000) {
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
	
#else
	
	(void)delta, (void)force;
	
#endif
	
}

void progress::set_enabled(bool enable) {
	show_progress = enable;
}
