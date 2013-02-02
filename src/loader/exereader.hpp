/*
 * Copyright (C) 2011-2012 Daniel Scharrer
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

#ifndef INNOEXTRACT_LOADER_EXEREADER_HPP
#define INNOEXTRACT_LOADER_EXEREADER_HPP

#include <istream>

#include <boost/cstdint.hpp>

namespace loader {

/*!
 * \brief Minimal PE/COFF parser that can find resources by ID in .exe files
 *
 * This implementation is optimized to look for exactly one resource.
 */
class exe_reader {
	
public:
	
	//! Position and size of a resource entry
	struct resource {
		
		boost::uint32_t offset; //!< File offset of the resource data in bytes
		
		boost::uint32_t size; //!< Size of the resource data in bytes
		
	};
	
	enum resource_id {
		TypeData = 10,
		LanguageDefault = 0
	};
	
	/*!
	 * \brief Find where a resource with a given ID is stored in a MS PE/COFF executable.
	 *
	 * Resources are addressed using a (\pname{name}, \pname{type}, \pname{language}) tuple.
	 *
	 * \param is       a seekable stream of the executable file containing the resource
	 * \param name     the user-defined name of the resource
	 * \param type     the type of the resource
	 * \param language the localised variant of the resource
	 *
	 * \return the location of the resource or `(0, 0)` if the requested resource does not exist.
	 */
	static resource find_resource(std::istream & is, boost::uint32_t name,
	                              boost::uint32_t type = TypeData,
	                              boost::uint32_t language = LanguageDefault);
	
};

} // namespace loader

#endif // INNOEXTRACT_LOADER_EXEREADER_HPP
