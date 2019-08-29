/*
 * Copyright (C) 2011-2019 Daniel Scharrer
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
 * Inno Setup version number utilities.
 */
#ifndef INNOEXTRACT_SETUP_VERSION_HPP
#define INNOEXTRACT_SETUP_VERSION_HPP

#include <iosfwd>
#include <exception>

#include <boost/cstdint.hpp>

#include "util/flags.hpp"

namespace setup {

struct version_error : public std::exception { };

typedef boost::uint32_t version_constant;
#define INNO_VERSION_EXT(a, b, c, d) ( \
	  (::setup::version_constant(a) << 24) \
	| (::setup::version_constant(b) << 16) \
	| (::setup::version_constant(c) <<  8) \
	| (::setup::version_constant(d) <<  0) \
)
#define INNO_VERSION(a, b, c) INNO_VERSION_EXT(a, b, c, 0)

struct version {
	
	FLAGS(flags,
		Bits16,
		Unicode,
		ISX
	);
	
	version_constant value;
	
	flags variant;
	
	bool known;
	
	version() : value(0), variant(0), known(false) { }
	
	version(version_constant v, flags type = 0, bool is_known = false)
		: value(v), variant(type), known(is_known) { }
	
	
	version(boost::uint8_t a, boost::uint8_t b, boost::uint8_t c, boost::uint8_t d = 0,
	        flags type = 0, bool is_known = false)
		: value(INNO_VERSION_EXT(a, b, c, d)), variant(type), known(is_known) { }
	
	unsigned int a() const { return  value >> 24;         }
	unsigned int b() const { return (value >> 16) & 0xff; }
	unsigned int c() const { return (value >>  8) & 0xff; }
	unsigned int d() const { return  value        & 0xff; }
	
	void load(std::istream & is);
	
	boost::uint16_t bits() const { return (variant & Bits16) ? 16 : 32; }
	bool is_unicode() const { return (variant & Unicode) != 0; }
	bool is_isx() const { return (variant & ISX) != 0; }
	
	//! \return true if the version stored might not be correct
	bool is_ambiguous() const;
	
	operator version_constant() const {
		return value;
	}
	
	version_constant next();
	
};

std::ostream & operator<<(std::ostream & os, const version & version);

} // namespace setup

#endif // INNOEXTRACT_SETUP_VERSION_HPP
