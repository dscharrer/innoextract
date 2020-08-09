/*
 * Copyright (C) 2012-2020 Daniel Scharrer
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
 * Compatibility functions for older Boost.Filesystem versions.
 */
#ifndef INNOEXTRACT_UTIL_BOOSTFS_COMPAT_HPP
#define INNOEXTRACT_UTIL_BOOSTFS_COMPAT_HPP

#include <string>

#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>

namespace util {

inline const std::string & as_string(const std::string & path) {
	return path;
}

inline const std::string as_string(const boost::filesystem::path & path) {
	return path.string();
}

} // namespace util

#endif // INNOEXTRACT_UTIL_BOOSTFS_COMPAT_HPP
