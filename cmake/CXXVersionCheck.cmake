
# Copyright (C) 2013-2016 Daniel Scharrer
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

include(CheckCXXSourceCompiles)
include(CompileCheck)

set(_HAS_CXX11 0)
set(CXX11_CHECK_DIR "${CMAKE_CURRENT_LIST_DIR}/check")

function(enable_cxx_version version)
	if(MSVC)
		if(NOT version LESS 2011 AND MSVC_VERSION LESS 1600)
			if(NOT version LESS 2017 AND NOT MSVC_VERSION LESS 1911)
				set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++17")
			elseif(NOT version LESS 2014 AND NOT MSVC_VERSION LESS 1910)
				set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++14")
			elseif(NOT version LESS 2014 AND NOT MSVC_VERSION LESS 1900)
				# Only introduced with update 3 of MSVC 2015
				add_cxxflag("/std:c++14")
			endif()
			set(_HAS_CXX11 1 PARENT_SCOPE)
		endif()
	else()
		set(FLAG_FOUND 0)
		if(NOT version LESS 2017)
			add_cxxflag("-std=c++17")
		endif()
		if(NOT version LESS 2014 AND NOT FLAG_FOUND)
			add_cxxflag("-std=c++14")
		endif()
		if(NOT version LESS 2011 AND NOT FLAG_FOUND)
			add_cxxflag("-std=c++11")
		endif()
		if(NOT version LESS 2011 AND FLAG_FOUND OR NOT CMAKE_COMPILER_IS_GNUCXX)
			if(SET_WARNING_FLAGS)
				add_cxxflag("-pedantic")
			endif()
			set(_HAS_CXX11 1 PARENT_SCOPE)
		endif()
	endif()
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}" PARENT_SCOPE)
endfunction(enable_cxx_version)

function(check_cxx11 CHECK RESULTVAR)
	if(_HAS_CXX11)
		if(MSVC AND ARGC GREATER 2)
			if(MSVC_VERSION LESS ARGV2)
				set(result)
			else()
				set(result 1)
			endif()
		else()
			string(REGEX REPLACE "[^a-zA-Z0-9_][^a-zA-Z0-9_]*" "-" check "${CHECK}")
			set(file "${CXX11_CHECK_DIR}/cxx11-${check}.cpp")
			set(old_CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
			set(old_CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS}")
			strip_warning_flags(CMAKE_CXX_FLAGS)
			strip_warning_flags(CMAKE_EXE_LINKER_FLAGS)
			check_compile(result "${file}" "${CHECK}" "C++11 feature")
			set(CMAKE_CXX_FLAGS "${old_CMAKE_CXX_FLAGS}")
			set(CMAKE_EXE_LINKER_FLAGS "${old_CMAKE_EXE_LINKER_FLAGS}")
		endif()
		if(NOT DEFINED result OR result STREQUAL "")
			set(${RESULTVAR} OFF PARENT_SCOPE)
		else()
			set(${RESULTVAR} ON PARENT_SCOPE)
		endif()
	else()
		set(${RESULTVAR} OFF PARENT_SCOPE)
	endif()
endfunction()
