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
 * Structures for Windows version numbers stored in Inno Setup files.
 */
#ifndef INNOEXTRACT_SETUP_WINDOWS_HPP
#define INNOEXTRACT_SETUP_WINDOWS_HPP

#include <iosfwd>

namespace setup {

struct version;

struct windows_version {
	
	struct data {
		
		unsigned major;
		unsigned minor;
		unsigned build;
		
		bool operator==(const data & o) const {
			return (build == o.build && major == o.major && minor == o.minor);
		}
		
		bool operator!=(const data & o) const {
			return !(*this == o);
		}
		
		void load(std::istream & is, const version & version);
		
	};
	
	data win_version;
	data nt_version;
	
	struct service_pack {
		
		unsigned major;
		unsigned minor;
	
		bool operator==(const service_pack & o) const {
			return (major == o.major && minor == o.minor);
		}
		
		bool operator!=(const service_pack & o) const {
			return !(*this == o);
		}
		
	};
	
	service_pack nt_service_pack;
	
	void load(std::istream & is, const version & version);
	
	bool operator==(const windows_version & o) const {
		return (win_version == o.win_version
		        && nt_version == o.nt_version
		        && nt_service_pack == o.nt_service_pack);
	}
	
	bool operator!=(const windows_version & o) const {
		return !(*this == o);
	}
	
	static const windows_version none;
	
};

struct windows_version_range {
	
	windows_version begin;
	windows_version end;
	
	void load(std::istream & is, const version & version);
	
};

std::ostream & operator<<(std::ostream & os, const windows_version::data & version);
std::ostream & operator<<(std::ostream & os, const windows_version & version);

} // namespace setup

#endif // INNOEXTRACT_SETUP_WINDOWS_HPP
