/*
 * Copyright (C) 2013-2014 Daniel Scharrer
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

/*!
 * \file
 *
 * Compatibility wrapper to work around deficiencies in Microsoft® Windows™.
 * Mostly deals with converting between UTF-8 and UTF-16 input/output.
 * More precisely:
 *  - Converts wide char command-line arguments to UTF-8 and calls utf8_main().
 *  - Sets an UTF-8 locale for boost::filesystem::path.
 *    This makes everything in boost::filesystem UTF-8 aware, except for {i,o,}fstream.
 *    For those, there are UTF-8 aware implementations in util/fstream.hpp
 *  - Converts UTF-8 to UTF-16 in std::cout and std::cerr if attached to a console
 *  - Interprets ANSI escape sequences in std::cout and std::cerr if attached to a console
 *  - Provides a Windows implementation of \ref isatty()
 */
#ifndef INNOEXTRACT_UTIL_WINDOWS_HPP
#define INNOEXTRACT_UTIL_WINDOWS_HPP

#if defined(_WIN32)

//! Program entry point that will always receive UTF-8 encoded arguments
int utf8_main(int argc, char * argv[]);

//! We define our own wrapper main(), so rename the real one
#define main utf8_main

namespace util {

//! isatty() replacement (only works for fd 0, 1 and 2)
int isatty(int fd);

//! Determine the buffer width of the current console - replacement for ioctl(TIOCGWINSZ)
int console_width();

} // namespace util

using util::isatty;

#endif // defined(_WIN32)

#endif // INNOEXTRACT_UTIL_WINDOWS_HPP
