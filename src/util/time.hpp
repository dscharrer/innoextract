/*
 * Copyright (C) 2011-2013 Daniel Scharrer
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

#ifndef INNOEXTRACT_UTIL_TIME_HPP
#define INNOEXTRACT_UTIL_TIME_HPP

#include <stdint.h>
#include <ctime>
#include <string>

#include <boost/filesystem/path.hpp>

namespace util {

/*!
 * Convert UTC clock time to a timestamp
 * Note: may not be threadsafe on all systems!
 */
std::time_t parse_time(std::tm tm);

/*!
 * Convert a timestamp to UTC clock time
 * Note: may not be threadsafe on all systems!
 */
std::tm format_time(std::time_t t);

/*!
 * Convert a timestamp to local time
 * Note: may not be threadsafe on all systems!
 */
std::time_t to_local_time(std::time_t t);

//! Set a file's creation/modification time
bool set_file_time(const boost::filesystem::path & path, std::time_t t, uint32_t nsec);

//! Set the local timezone used by to_local_time
void set_local_timezone(std::string timezone);

} // namespace util

#endif // INNOEXTRACT_UTIL_TIME_HPP
