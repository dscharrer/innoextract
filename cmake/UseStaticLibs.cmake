
# Copyright (C) 2013-2019 Daniel Scharrer
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

macro(use_static_libs ID)
	if(${ID}_USE_STATIC_LIBS)
		set(_UseStaticLibs_ORIG_CMAKE_FIND_LIBRARY_SUFFIXES ${CMAKE_FIND_LIBRARY_SUFFIXES})
		if(WIN32)
			set(CMAKE_FIND_LIBRARY_SUFFIXES _a.lib .lib .a)
		else()
			set(CMAKE_FIND_LIBRARY_SUFFIXES .a)
		endif()
		if(ARGC GREATER 1)
			set(prefix "${ARGV1}")
			set(${prefix}_LIBRARIES     "${${prefix}_STATIC_LIBRARIES}")
			set(${prefix}_LIBRARY_DIRS  "${${prefix}_STATIC_LIBRARY_DIRS}")
			set(${prefix}_LDFLAGS       "${${prefix}_STATIC_LDFLAGS}")
			set(${prefix}_LDFLAGS_OTHER "${${prefix}_STATIC_LDFLAGS_OTHER}")
			set(${prefix}_INCLUDE_DIRS  "${${prefix}_STATIC_INCLUDE_DIRS}")
			set(${prefix}_CFLAGS        "${${prefix}_STATIC_CFLAGS}")
			set(${prefix}_CFLAGS_OTHER  "${${prefix}_STATIC_CFLAGS_OTHER}")
		endif()
	endif()
endmacro()

macro(use_static_libs_restore)
	if(DEFINED _UseStaticLibs_ORIG_CMAKE_FIND_LIBRARY_SUFFIXES)
		set(CMAKE_FIND_LIBRARY_SUFFIXES ${_UseStaticLibs_ORIG_CMAKE_FIND_LIBRARY_SUFFIXES})
		unset(_UseStaticLibs_ORIG_CMAKE_FIND_LIBRARY_SUFFIXES)
	endif()
endmacro()

macro(has_static_libs PREFIX LIBS)
	if(WIN32)
		# On Windows we can't really tell import libraries from proper static libraries.
		set(${PREFIX}_HAS_STATIC_LIBS ${${PREFIX}_USE_STATIC_LIBS})
	else()
		set(${PREFIX}_HAS_STATIC_LIBS 0)
		foreach(lib IN LISTS ${LIBS})
			if(lib MATCHES "\\.a$")
				set(${PREFIX}_HAS_STATIC_LIBS 1)
				break()
			endif()
		endforeach()
	endif()
endmacro()
