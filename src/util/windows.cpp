/*
 * Copyright (C) 2013 Daniel Scharrer
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

#include <locale>
#include <clocale>

#include <windows.h>

#include <boost/filesystem/path.hpp>

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

// We really want main here, not utf8_main.
#undef main
int main() {
	
	// We use UTF-8 for everything internally, as almost all modern operating systems
	// have standardized on that. However, as usual, Windows has to do its own thing
	// and only supports Unicode input/output via UCS-2^H^H^H^H^HUTF-16.
	
	std::setlocale(LC_ALL, "");
	
	// Get the UTF-16 command-line parameters and convert it them to UTF-8 ourself.
	int argc = __argc;
	wchar_t ** wargv = __wargv;
	char ** argv = new char *[argc + 1];
	argv[argc] = NULL;
	for(int i = 0; i < argc; i++) {
		int n = WideCharToMultiByte(CP_UTF8, 0, wargv[i], -1, NULL, 0,  NULL, NULL);
		argv[i] = new char[n];
		WideCharToMultiByte(CP_UTF8, 0, wargv[i], -1, argv[i], n, NULL, NULL);
	}
	
	// Tell boost::filesystem to interpret our path strings as UTF-8.
	std::locale global_locale = std::locale();
	std::locale utf8_locale(global_locale, new utf8_codecvt);
	boost::filesystem::path::imbue(utf8_locale);
	
	return utf8_main(argc, argv);
}
