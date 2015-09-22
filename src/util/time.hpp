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
 * Time parsing, formatting, onversion and filetime manipulation functions.
 */
#ifndef INNOEXTRACT_UTIL_TIME_HPP
#define INNOEXTRACT_UTIL_TIME_HPP

#include <ctime>
#include <string>

#include <boost/cstdint.hpp>
#include <boost/filesystem/path.hpp>

namespace util {

typedef boost::int64_t time;

/*!
 * Convert UTC clock time to a timestamp
 *
 * \note This function may not be thread-safe on all operating systems.
 */
time parse_time(std::tm tm);

/*!
 * Convert a timestamp to UTC clock time
 *
 * \note This function may not be thread-safe on all operating systems.
 */
std::tm format_time(time t);

/*!
 * Convert a timestamp to local time
 *
 * \note This function may not be thread-safe on all operating systems.
 */
time to_local_time(time t);

/*!
 * Set the local timezone used by to_local_time
 *
 * \note This function is not thread-safe.
 */
void set_local_timezone(std::string timezone);

/*!
 * Set a file's access, creation and modification times.
 *
 * \note Not all operating systems support file creation times.
 *
 * \param path The file to the file to set the times for.
 *             This file will <b>not</b> be created if it doesn't exist already.
 * \param sec  File time to set (in seconds).
 * \param nsec Sub-second component of the file time to set (in nanoseconds).
 *
 * \note The actual file time precision depends on the operating system and filesystem.
 *       If the available file time precision is too low to represent the given timestamp,
 *       it will be silently truncated to the available precision.
 *
 * \return \c true if the file time was changed, \c false otherwise.
 */
bool set_file_time(const boost::filesystem::path & path, time sec, boost::uint32_t nsec);

} // namespace util

#endif // INNOEXTRACT_UTIL_TIME_HPP
