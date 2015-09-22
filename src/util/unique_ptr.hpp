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
 * Wrapper to select std::unique_ptr if available, std::auto_ptr otherwise.
 */
#ifndef INNOEXTRACT_UTIL_UNIQUE_PTR_HPP
#define INNOEXTRACT_UTIL_UNIQUE_PTR_HPP

#include <memory>

#include "configure.hpp"

namespace util {

//! Get a std::unique_ptr or std::auto_ptr for the given type.
template <typename T>
struct unique_ptr {
#if INNOEXTRACT_HAVE_STD_UNIQUE_PTR
	typedef std::unique_ptr<T> type;
#else
	typedef std::auto_ptr<T> type;
#endif
};

} // namespace util

#endif // INNOEXTRACT_UTIL_UNIQUE_PTR_HPP
