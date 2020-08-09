/*
 * Copyright (C) 2013-2020 Daniel Scharrer
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

#ifndef _WIN32
#define _WIN32
#endif
#include "util/windows.hpp"

#include <stddef.h>

#include <algorithm>
#include <cstdlib>
#include <clocale>
#include <iostream>
#include <locale>
#include <sstream>
#include <stdexcept>
#include <vector>

#include <wchar.h>
#include <windows.h>
#include <shellapi.h>

#include <boost/filesystem/path.hpp>
#include <boost/iostreams/stream_buffer.hpp>

#include "configure.hpp"

#if INNOEXTRACT_HAVE_STD_CODECVT_UTF8_UTF16
// C++11
#include <codecvt>
namespace { typedef std::codecvt_utf8_utf16<wchar_t> utf8_codecvt; }
#else
// Using private Boost stuff - bad, but meh.
#include <boost/filesystem/detail/utf8_codecvt_facet.hpp>
namespace { typedef boost::filesystem::detail::utf8_codecvt_facet utf8_codecvt; }
#endif

#include "util/ansi.hpp"
#include "util/encoding.hpp"

// Disable telemetry added in Visual Studio 2015
#if defined(_MSC_VER) && _MSC_VER >= 1900
extern "C" {
	void _cdecl __vcrt_initialize_telemetry_provider() { }
	void _cdecl __telemetry_main_invoke_trigger() { }
	void _cdecl __telemetry_main_return_trigger() { }
	void _cdecl __vcrt_uninitialize_telemetry_provider() { }
};
#endif

namespace util {

class windows_console_sink : public util::ansi_console_parser<windows_console_sink> {
	
	friend class util::ansi_console_parser<windows_console_sink>;
	
	const HANDLE handle;
	
	//! Buffer for charset conversions
	std::string in_buffer;
	std::string out_buffer;
	
	//! Initial console display attributes
	WORD initial_attributes;
	//! Default console display attributes
	WORD default_attributes;
	//! Current console display attributes
	WORD attributes;
	
	bool deferred_clear;
	SHORT clear_line;
	WORD clear_attributes;
	
	void clear_deferred(const CONSOLE_SCREEN_BUFFER_INFO & info, SHORT offset = 0) {
		COORD pos = { offset, clear_line };
		DWORD count = DWORD(info.dwSize.X - offset);
		DWORD ignored;
		FillConsoleOutputCharacterW(handle, L' ', count, pos, &ignored);
		FillConsoleOutputAttribute(handle, clear_attributes, count, pos, &ignored);
		deferred_clear = false;
	}
	
	void erase_in_line(const char * codes, const char * end) {
		
		bool left = false, right = false;
		bool deferred = false;
		
		do {
			unsigned code = read_code(codes, end);
			switch(code) {
				case 0:              right = true; break;
				case 1: left = true;               break;
				case 2: left = true, right = true; break;
				case 3: deferred = true;           break;
				default: {
					#ifdef DEBUG
					std::ostringstream oss;
					oss << "(unsupported EL code: " << code << ")";
					error(oss.str());
					#endif
				}
			}
		} while(codes);
		
		CONSOLE_SCREEN_BUFFER_INFO info;
		if(!GetConsoleScreenBufferInfo(handle, &info)) {
			return;
		}
		
		if(deferred_clear && (!deferred
		   || clear_line != info.dwCursorPosition.Y || clear_attributes != attributes)) {
			clear_deferred(info);
		}
		
		if(deferred) {
			deferred_clear = true;
			clear_line = info.dwCursorPosition.Y;
			clear_attributes = attributes;
			return;
		}
		
		SHORT cbegin = left ? SHORT(0) : info.dwCursorPosition.X;
		SHORT cend = right ? info.dwSize.X : info.dwCursorPosition.X;
		
		COORD pos = { cbegin, info.dwCursorPosition.Y };
		DWORD count = DWORD(cend - cbegin);
		
		DWORD ignored;
		FillConsoleOutputCharacterW(handle, L' ', count, pos, &ignored);
		FillConsoleOutputAttribute(handle, attributes, count, pos, &ignored);
	}
	
	void set_attributes(WORD attr) {
		if(attributes != attr) {
			attributes = attr;
			SetConsoleTextAttribute(handle, attributes);
		}
	}
	
	WORD get_attributes() {
		
		CONSOLE_SCREEN_BUFFER_INFO info;
		if(!GetConsoleScreenBufferInfo(handle, &info)) {
			return FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_INTENSITY;
		}
		
		return info.wAttributes;
	}
	
	WORD get_default_attributes() {
		
		int a = initial_attributes;
		
		// Unset onderscore and negative states
		a &= ~(COMMON_LVB_REVERSE_VIDEO | COMMON_LVB_UNDERSCORE);
		
		// Set dim white text, otherwise the default color might clash with other colors
		a |= FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED;
		a &= ~FOREGROUND_INTENSITY;
		
		// Try to preserve the existing background color if it is dark enough
		const WORD bg = BACKGROUND_BLUE | BACKGROUND_GREEN | BACKGROUND_RED;
		a &= ~BACKGROUND_INTENSITY;
		if((a & bg) == bg) {
			a &= ~bg; // Prevent white text on white background
		}
		
		return WORD(a);
	}
	
	void select_graphic_rendition(const char * codes, const char * end) {
		
		const WORD fg = FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED;
		const WORD bg = BACKGROUND_BLUE | BACKGROUND_GREEN | BACKGROUND_RED;
		
		int a = attributes;
		
		do {
			unsigned code = read_code(codes, end);
			switch(code) {
				case 0: a = default_attributes; break;
				case 1: a |= FOREGROUND_INTENSITY; break;
				case 4: a |= COMMON_LVB_UNDERSCORE; break;
				case 7: a |= COMMON_LVB_REVERSE_VIDEO; break;
				case 22: a &= ~FOREGROUND_INTENSITY; break;
				case 24: a &= ~COMMON_LVB_UNDERSCORE; break;
				case 27: a &= ~COMMON_LVB_REVERSE_VIDEO; break;
				case 30: a &= ~fg; break;
				case 31: a &= ~fg, a |= FOREGROUND_RED; break;
				case 32: a &= ~fg, a |= FOREGROUND_GREEN; break;
				case 33: a &= ~fg, a |= FOREGROUND_RED | FOREGROUND_GREEN; break;
				case 34: a &= ~fg, a |= FOREGROUND_BLUE; break;
				case 35: a &= ~fg, a |= FOREGROUND_RED | FOREGROUND_BLUE; break;
				case 36: a &= ~fg, a |= FOREGROUND_BLUE | FOREGROUND_GREEN; break;
				case 37: a &= ~fg, a |= FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE; break;
				case 39: a &= ~fg, a |= (default_attributes & fg); break;
				case 40: a &= ~bg; break;
				case 41: a &= ~bg, a |= BACKGROUND_RED; break;
				case 42: a &= ~bg, a |= BACKGROUND_GREEN; break;
				case 43: a &= ~bg, a |= BACKGROUND_RED | BACKGROUND_GREEN; break;
				case 44: a &= ~bg, a |= BACKGROUND_BLUE; break;
				case 45: a &= ~bg, a |= BACKGROUND_RED | BACKGROUND_BLUE; break;
				case 46: a &= ~bg, a |= BACKGROUND_BLUE | BACKGROUND_GREEN; break;
				case 47: a &= ~bg, a |= BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE; break;
				case 49: a &= ~bg, a |= (default_attributes & bg); break;
				default: {
					#ifdef DEBUG
					std::ostringstream oss;
					oss << "(unsupported SGR code: " << code << ")";
					error(oss.str());
					#endif
				}
			}
		} while(codes);
		
		set_attributes(WORD(a));
	}
	
	void handle_command(CommandType type, const char * codes, const char * end) {
		if(!in_buffer.empty()) {
			output_text(in_buffer.c_str(), in_buffer.c_str() + in_buffer.size());
			in_buffer.clear();
		}
		switch(type) {
			case EL:  erase_in_line(codes, end); break;
			case SGR: select_graphic_rendition(codes, end); break;
			default: {
				#ifdef DEBUG
				std::ostringstream oss;
				oss << "(unknown command type: " << char(type) << ")";
				error(oss.str());
				#endif
			}
		}
	}
	
	void handle_deferred_clear(LPCWSTR & begin, LPCWSTR end) {
		
		CONSOLE_SCREEN_BUFFER_INFO info;
		if(!GetConsoleScreenBufferInfo(handle, &info)) {
			deferred_clear = false;
			return;
		}
		
		while(begin != end) {
			
			if(*begin == L'\r') {
				// End deferred clear mode
				deferred_clear = false;
				break;
			}
			
			LPCWSTR cr = std::find(begin, end, L'\r');
			LPCWSTR ln = std::find(begin, cr, L'\n');
			
			// Insert an empty line before the "cleared" line
			if(clear_line == info.dwCursorPosition.Y) {
				
				if(info.dwCursorPosition.Y == info.dwSize.Y - 1) {
					// Cursor is at the end of the buffer
					// Move buffer contents up one line except for the last line
					SMALL_RECT source = { 0, 1, SHORT(info.dwSize.X), SHORT(info.dwSize.Y - 2) };
					COORD dest = { 0, 0 };
					CHAR_INFO fill;
					fill.Char.UnicodeChar = L' ';
					fill.Attributes = clear_attributes;
					ScrollConsoleScreenBufferW(handle, &source, NULL, dest, &fill);
					COORD cursor = { 0, SHORT(info.dwCursorPosition.Y - 1) };
					SetConsoleCursorPosition(handle, cursor);
				} else {
					// Move cleared line down one line
					SMALL_RECT source = { 0, SHORT(info.dwCursorPosition.Y),
					                      SHORT(info.dwSize.X), SHORT(info.dwCursorPosition.Y + 1) };
					SMALL_RECT clip = { 0, SHORT(info.dwCursorPosition.Y + 1),
					                    SHORT(info.dwSize.X), SHORT(info.dwCursorPosition.Y + 2) };
					COORD dest = { 0, SHORT(info.dwCursorPosition.Y + 1) };
					CHAR_INFO fill;
					fill.Char.UnicodeChar = L' ';
					fill.Attributes = clear_attributes;
					ScrollConsoleScreenBufferW(handle, &source, &clip, dest, &fill);
					clear_line = SHORT(info.dwCursorPosition.Y + 1);
					if(info.dwCursorPosition.Y == info.srWindow.Bottom) {
						// Cursor is at the end of the window
						// Scroll up before overwriting the cleared line
						SMALL_RECT window = { 0, 1, 0, 1 };
						SetConsoleWindowInfo(handle, FALSE, &window);
					}
					COORD pos = { 0, info.dwCursorPosition.Y };
					DWORD count = DWORD(info.dwSize.X);
					DWORD ignored;
					FillConsoleOutputCharacterW(handle, L' ', count, pos, &ignored);
					FillConsoleOutputAttribute(handle, clear_attributes, count, pos, &ignored);
				}
				
				info.dwCursorPosition.X = 0;
				
			}
			
			// Write at most one line!
			DWORD len = DWORD(std::min(ln + 1 - begin, cr - begin));
			len = std::min(len, DWORD(info.dwSize.X - info.dwCursorPosition.X));
			
			DWORD count;
			WriteConsoleW(handle, begin, len, &count, NULL);
			begin += len;
			
			if(!GetConsoleScreenBufferInfo(handle, &info)) {
				deferred_clear = false;
				break;
			}
			
			if(info.dwCursorPosition.Y > clear_line) {
				// Line completely overwritten with text
				deferred_clear = false;
				break;
			} else if(info.dwCursorPosition.Y == clear_line && info.dwCursorPosition.X > 0) {
				// Line partially overwritten with text - clear the rest
				clear_deferred(info, info.dwCursorPosition.X);
				break;
			}
			
		}
		
	}
	
	void output_text(const char * begin, const char * end) {
		
		wtf8_to_utf16le(begin, end, out_buffer);
		
		LPCWSTR obegin = reinterpret_cast<LPCWSTR>(out_buffer.c_str());
		LPCWSTR oend = obegin + out_buffer.size() / 2;
		
		if(deferred_clear) {
			handle_deferred_clear(obegin, oend);
		}
		
		DWORD count;
		WriteConsoleW(handle, obegin, DWORD(oend - obegin), &count, NULL);
		
	}
	
	void handle_text(const char * s, size_t n) {
		
		if(!in_buffer.empty()) {
			in_buffer.append(s, n);
			s = in_buffer.c_str();
			n = in_buffer.size();
		}
		
		const char * end = s + n;
		
		if(s == end) {
			return;
		}
		
		const char * e = wtf8_find_end(s, end);
		
		output_text(s, e);
		
		in_buffer.assign(e, end);
		
	}
	
public:
	
	explicit windows_console_sink(HANDLE console_handle)
		: handle(console_handle)
		, initial_attributes(get_attributes())
		, default_attributes(get_default_attributes())
		, attributes(initial_attributes)
		, deferred_clear(false)
		, clear_line(0)
		, clear_attributes(0)
	{ }
	
	~windows_console_sink() {
		if(!in_buffer.empty()) {
			output_text(in_buffer.c_str(), in_buffer.c_str() + in_buffer.size());
		}
		if(deferred_clear) {
			CONSOLE_SCREEN_BUFFER_INFO info;
			if(GetConsoleScreenBufferInfo(handle, &info)) {
				clear_deferred(info);
			}
		}
		set_attributes(initial_attributes);
	}
	
};

typedef boost::iostreams::stream_buffer<windows_console_sink> console_buffer;
struct console_buffer_info {
	HANDLE handle;
	console_buffer * buf;
	std::streambuf * orig_buf;
};
static console_buffer_info stdout_info = { NULL, NULL, NULL };
static console_buffer_info stderr_info = { NULL, NULL, NULL };

static void cleanup_console(std::ostream & os, console_buffer_info & info) {
	if(info.buf) {
		os.flush();
		os.rdbuf(info.orig_buf);
		delete info.buf;
		info.buf = NULL, info.handle = NULL;
	}
}

static void cleanup_console() {
	cleanup_console(std::cout, stdout_info);
	cleanup_console(std::cerr, stderr_info);
}

static BOOL WINAPI cleanup_console_handler(DWORD type) {
	(void)type;
	cleanup_console();
	return FALSE;
}

static bool is_console(HANDLE handle) {
	DWORD mode;
	return GetConsoleMode(handle, &mode) != 0;
}

static void init_console(std::ostream & os, console_buffer_info & info, DWORD n) {
	info.handle = GetStdHandle(n);
	if(is_console(info.handle)) {
		info.buf = new console_buffer(info.handle);
		info.orig_buf = os.rdbuf(info.buf);
	} else {
		info.handle = NULL;
	}
}

static void init_console() {
	init_console(std::cout, stdout_info, STD_OUTPUT_HANDLE);
	init_console(std::cerr, stderr_info, STD_ERROR_HANDLE);
	if(stdout_info.buf || stderr_info.buf) {
		std::atexit(cleanup_console);
		SetConsoleCtrlHandler(cleanup_console_handler, TRUE);
	}
}

struct console_wrapper {
	
	console_wrapper() {
		init_console();
	}
	
	~console_wrapper() {
		cleanup_console();
	}
	
};

int isatty(int fd) {
	switch(fd) {
		case 0: return is_console(GetStdHandle(STD_INPUT_HANDLE)) ? 1 : 0;
		case 1: return (stdout_info.buf != NULL) ? 1 : 0;
		case 2: return (stderr_info.buf != NULL) ? 1 : 0;
		default: return 0;
	}
}

int console_width() {
	
	if(!stdout_info.handle) {
		return 0;
	}
	
	CONSOLE_SCREEN_BUFFER_INFO info;
	if(!GetConsoleScreenBufferInfo(stdout_info.handle, &info)) {
		return 0;
	}
	
	return int(info.dwSize.X);
}

} // namespace util

// We really want main here, not utf8_main.
#undef main
int main() {
	
	// We use UTF-8 for everything internally, as almost all modern operating systems
	// have standardized on that. However, as usual, Windows has to do its own thing
	// and only supports Unicode input/output via UCS-2^H^H^H^H^HUTF-16.
	
	std::setlocale(LC_ALL, "");
	
	// Emulate wmain() as it's nonstandard and not supported by MinGW
	// Convert the UTF-16 command-line parameters to UTF-8
	int argc = 0;
	char ** argv = NULL;
	std::vector<std::string> args;
	{
		wchar_t ** wargv = CommandLineToArgvW(GetCommandLineW(), &argc);
		args.resize(size_t(argc));
		argv = new char *[argc + 1];
		argv[argc] = NULL;
		for(size_t i = 0; i < args.size(); i++) {
			util::utf16le_to_wtf8(std::string(reinterpret_cast<char *>(wargv[i]), wcslen(wargv[i]) * 2), args[i]);
			argv[i] = &args[i][0];
		}
		LocalFree(wargv);
	}
	
	// Tell boost::filesystem to interpret our path strings as UTF-8
	boost::filesystem::path::imbue(std::locale(std::locale(), new utf8_codecvt()));
	
	// Enable UTF-8 output and ANSI escape sequences
	util::console_wrapper wrapped;
	
	return utf8_main(argc, argv);
}
