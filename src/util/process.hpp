/*
 * Copyright (C) 2013-2015 Daniel Scharrer
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

#ifndef INNOEXTRACT_UTIL_PROCESS_HPP
#define INNOEXTRACT_UTIL_PROCESS_HPP

#include <string>

namespace util {

/*!
 * \brief Start a program and wait for it to finish
 *
 * The executable's standard output/error is discarded.
 *
 * \param args program arguments. The first argument is the program name/path and
 *             the last argument must be NULL.
 *
 * \return the programs exit code or a negative value on error.
 */
int run(const char * const args[]);

} // namespace util

#endif // INNOEXTRACT_UTIL_PROCESS_HPP
