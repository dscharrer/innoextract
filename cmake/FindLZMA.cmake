
# Copyright (C) 2011 Daniel Scharrer
#
# This software is provided 'as-is', without any express or implied
# warranty.  In no event will the author(s) be held liable for any damages
# arising from the use of this software.
#
# Permission is granted to anyone to use this software for any purpose,
# including commercial applications, and to alter it and redistribute it
# freely, subject to the following restrictions:
#
# 1. The origin of this software must not be misrepresented; you must not
#    claim that you wrote the original software. If you use this software
#    in a product, an acknowledgment in the product documentation would be
#    appreciated but is not required.
# 2. Altered source versions must be plainly marked as such, and must not be
#    misrepresented as being the original software.
# 3. This notice may not be removed or altered from any source distribution.

# Try to find the LZMA library and include path for lzma.h from xz-utils.
# Once done this will define
#
# LZMA_FOUND
# LZMA_INCLUDE_DIR - where to find lzma.h
# LZMA_LIBRARIES - liblzma.so

option(LZMA_USE_STATIC_LIBS "Statically link liblzma" OFF)

include(UseStaticLibs)
use_static_libs(LZMA)

find_path(LZMA_INCLUDE_DIR lzma.h DOC "The directory where lzma.h resides")
find_library(LZMA_LIBRARY lzma DOC "The LZMA library")

mark_as_advanced(LZMA_INCLUDE_DIR)
mark_as_advanced(LZMA_LIBRARY)

use_static_libs_restore()

# handle the QUIETLY and REQUIRED arguments and set LZMA_FOUND to TRUE if 
# all listed variables are TRUE
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(LZMA DEFAULT_MSG LZMA_LIBRARY LZMA_INCLUDE_DIR)

if(LZMA_FOUND)
	set(LZMA_LIBRARIES ${LZMA_LIBRARY})
endif(LZMA_FOUND)
