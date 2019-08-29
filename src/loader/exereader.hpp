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
 * Functions to find resources in Windows executables.
 */
#ifndef INNOEXTRACT_LOADER_EXEREADER_HPP
#define INNOEXTRACT_LOADER_EXEREADER_HPP

#include <istream>

#include <boost/cstdint.hpp>

namespace loader {

/*!
 * \brief Minimal NE/LE/PE parser that can find resources by ID in binary (exe/dll) files
 *
 * This implementation is optimized to look for exactly one resource.
 */
class exe_reader {
	
public:
	
	//! Position and size of a resource entry
	struct resource {
		
		boost::uint32_t offset; //!< File offset of the resource data in bytes
		
		boost::uint32_t size; //!< Size of the resource data in bytes
		
		operator bool() { return offset != 0; }
		bool operator!() { return offset == 0; }
		
	};
	
	enum resource_id {
		NameVersionInfo = 1,
		TypeCursor = 1,
		TypeBitmap = 2,
		TypeIcon = 3,
		TypeMenu = 4,
		TypeDialog = 5,
		TypeString = 6,
		TypeFontDir = 7,
		TypeFont = 8,
		TypeAccelerator = 9,
		TypeData = 10,
		TypeMessageTable = 11,
		TypeGroupCursor = 12,
		TypeGroupIcon = 14,
		TypeVersion = 16,
		TypeDlgInclude = 17,
		TypePlugPlay = 19,
		TypeVXD = 20,
		TypeAniCursor = 21,
		TypeAniIcon = 22,
		TypeHTML = 23,
		Default = boost::uint32_t(-1)
	};
	
	/*!
	 * \brief Find where a resource with a given ID is stored in a NE or PE binary.
	 *
	 * Resources are addressed using a (name, type, language) tuple.
	 *
	 * \param is       a seekable stream of the binary containing the resource
	 * \param name     the user-defined name of the resource
	 * \param type     the type of the resource
	 * \param language the localised variant of the resource
	 *
	 * \return the location of the resource or `(0, 0)` if the requested resource does not exist.
	 */
	static resource find_resource(std::istream & is, boost::uint32_t name,
	                              boost::uint32_t type = TypeData,
	                              boost::uint32_t language = Default);
	
	enum file_version {
		FileVersionUnknown = boost::uint64_t(-1)
	};
	
	/*!
	 * \brief Get the file version number of a NE, LE or PE binary.
	 *
	 * \param is a seekable stream of the binary file containing the resource
	 *
	 * \return the file version number or FileVersionUnknown.
	 */
	static boost::uint64_t get_file_version(std::istream & is);
	
};

} // namespace loader

#endif // INNOEXTRACT_LOADER_EXEREADER_HPP
