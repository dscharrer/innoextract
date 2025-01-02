
# Copyright (C) 2011-2019 Daniel Scharrer
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
# LZMA_INCLUDE_DIR   Where to find lzma.h
# LZMA_LIBRARIES     The liblzma library
# LZMA_DEFINITIONS   Definitions to use when compiling code that uses liblzma
#
# Typical usage could be something like:
#   find_package(LZMA REQUIRED)
#   include_directories(SYSTEM ${LZMA_INCLUDE_DIR})
#   add_definitions(${LZMA_DEFINITIONS})
#   ...
#   target_link_libraries(myexe ${LZMA_LIBRARIES})
#
# The following additional options can be defined before the find_package() call:
# LZMA_USE_STATIC_LIBS  Statically link against liblzma (default: OFF)

if(UNIX)
	find_package(PkgConfig QUIET)
	pkg_check_modules(_PC_LZMA liblzma)
endif()

include(UseStaticLibs)

foreach(static IN ITEMS 1 0)
	
	if(static)
		use_static_libs(LZMA _PC_LZMA)
	endif()
	
	find_path(LZMA_INCLUDE_DIR lzma.h
		HINTS
			${_PC_LZMA_INCLUDE_DIRS}
		DOC "The directory where lzma.h resides"
	)
	mark_as_advanced(LZMA_INCLUDE_DIR)
	
	# Prefer libraries in the same prefix as the include files
	string(REGEX REPLACE "(.*)/include/?" "\\1" LZMA_BASE_DIR ${LZMA_INCLUDE_DIR})
	
	find_library(LZMA_LIBRARY lzma liblzma
		PATHS
			${_PC_LZMA_LIBRARY_DIRS}
			"${LZMA_BASE_DIR}/lib"
		DOC "The LZMA library"
	)
	mark_as_advanced(LZMA_LIBRARY)
	
	if(static)
		use_static_libs_restore()
	endif()
	
	if(LZMA_LIBRARY OR STRICT_USE)
		break()
	endif()
	
endforeach()

set(LZMA_DEFINITIONS)
if(WIN32 AND LZMA_USE_STATIC_LIBS)
	set(LZMA_DEFINITIONS -DLZMA_API_STATIC)
endif()

# handle the QUIETLY and REQUIRED arguments and set LZMA_FOUND to TRUE if 
# all listed variables are TRUE
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(LZMA DEFAULT_MSG LZMA_LIBRARY LZMA_INCLUDE_DIR)

if(LZMA_FOUND)
	set(LZMA_LIBRARIES ${LZMA_LIBRARY})
endif(LZMA_FOUND)
