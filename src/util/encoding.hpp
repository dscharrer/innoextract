/*
 * Copyright (C) 2011-2014 Daniel Scharrer
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
 * Utility function to convert strings to UTF-8.
 */
#ifndef INNOEXTRACT_UTIL_ENCODING_HPP
#define INNOEXTRACT_UTIL_ENCODING_HPP

#include <string>

#include <boost/cstdint.hpp>

namespace util {

typedef boost::uint32_t codepage_id;

/*!
 * Convert a string to UTF-8 from a specified encoding.
 * \param from     The input string to convert.
 * \param to       The output for the converted string.
 * \param codepage The Windows codepage number for the input string encoding.
 *
 * \note This function is not thread-safe.
 */
void to_utf8(const std::string & from, std::string & to, codepage_id codepage = 1252);

std::string encoding_name(codepage_id codepage);

} // namespace util

#endif // INNOEXTRACT_UTIL_ENCODING_HPP
