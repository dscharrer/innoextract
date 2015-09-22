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
 * boost::filesystem::{i,o,}fstream doesn't support unicode names on windows
 * Implement our own wrapper using boost::iostreams.
 */
#ifndef INNOEXTRACT_UTIL_FSTREAM_HPP
#define INNOEXTRACT_UTIL_FSTREAM_HPP

#if !defined(_WIN32)

#include <boost/filesystem/fstream.hpp>

namespace util {

typedef boost::filesystem::ifstream ifstream;
typedef boost::filesystem::ofstream ofstream;
typedef boost::filesystem::fstream  fstream;

} // namespace util

#else // if defined(_WIN32)

#include <boost/filesystem/path.hpp>
#include <boost/iostreams/device/file_descriptor.hpp>
#include <boost/iostreams/stream.hpp>

namespace util {

/*!
 * {i,o,}fstream implementation with support for Unicode filenames.
 * Create a subclass instead of a typedef to force boost::filesystem::path parameters.
 */
template <typename Device>
class path_fstream : public boost::iostreams::stream<Device> {
	
private: // disallow copying
	
	path_fstream(const path_fstream &);
	const path_fstream & operator=(const path_fstream &);
	
	typedef boost::filesystem::path path;
	typedef boost::iostreams::stream<Device> base;
	
	Device & device() { return **this; }
	
	void fix_open_mode(std::ios_base::openmode mode);
	
public:
	
	path_fstream() : base(Device()) { }
	
	explicit path_fstream(const path & p) : base(p) { }
	
	path_fstream(const path & p, std::ios_base::openmode mode) : base(p, mode) {
		fix_open_mode(mode);
	}
	
	void open(const path & p) {
		base::close();
		base::open(p);
	}

	void open(const path & p, std::ios_base::openmode mode) {
		base::close();
		base::open(p, mode);
		fix_open_mode(mode);
	}
	
	bool is_open() {
		return device().is_open(); // return the real open state, not base::is_open()
	}
	
	virtual ~path_fstream() { }
};

template <>
inline void path_fstream<boost::iostreams::file_descriptor_source>
	::fix_open_mode(std::ios_base::openmode mode) {
	if((mode & std::ios_base::ate) && is_open()) {
		seekg(0, std::ios_base::end);
	}
}

template <>
inline void path_fstream<boost::iostreams::file_descriptor_sink>
	::fix_open_mode(std::ios_base::openmode mode) {
	if((mode & std::ios_base::ate) && is_open()) {
		seekp(0, std::ios_base::end);
	}
}

template <>
inline void path_fstream<boost::iostreams::file_descriptor>
	::fix_open_mode(std::ios_base::openmode mode) {
	if((mode & std::ios_base::ate) && is_open()) {
		seekg(0, std::ios_base::end);
		seekp(0, std::ios_base::end);
	}
}

typedef path_fstream<boost::iostreams::file_descriptor_source> ifstream;
typedef path_fstream<boost::iostreams::file_descriptor_sink>   ofstream;
typedef path_fstream<boost::iostreams::file_descriptor>        fstream;

} // namespace util

#endif // defined(_WIN32)

#endif // INNOEXTRACT_UTIL_FSTREAM_HPP
