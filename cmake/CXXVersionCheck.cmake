
# Copyright (C) 2013-2018 Daniel Scharrer
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

set(CXX_VERSION 2003)
set(CXX_CHECK_DIR "${CMAKE_CURRENT_LIST_DIR}/check")

function(enable_cxx_version version)
	
	set(versions 17 14 11)
	
	if(MSVC)
		if(NOT version LESS 2011 AND NOT MSVC_VERSION LESS 1600)
			set(CXX_VERSION 2011)
			if(NOT version LESS 2017 AND NOT MSVC_VERSION LESS 1911)
				set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++17")
				set(CXX_VERSION 2017)
			elseif(NOT version LESS 2014 AND NOT MSVC_VERSION LESS 1910)
				set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++14")
				set(CXX_VERSION 2014)
			elseif(NOT version LESS 2014 AND NOT MSVC_VERSION LESS 1900)
				# Only introduced with update 3 of MSVC 2015
				add_cxxflag("/std:c++14")
				if(FLAG_FOUND)
					set(CXX_VERSION 2014)
				endif()
			endif()
		endif()
	else()
		set(FLAG_FOUND 0)
		foreach(ver IN LISTS versions)
			if(NOT version LESS 20${ver} AND NOT FLAG_FOUND)
				add_cxxflag("-std=c++${ver}")
				if(FLAG_FOUND)
					set(CXX_VERSION 20${ver})
					break()
				endif()
			endif()
		endforeach()
		if(NOT FLAG_FOUND)
			# Check if the compiler supports the -std flag at all
			# Don't actually use the flag to allow for compiler extensions a la -sdt=gnu++03
			check_compiler_flag(FLAG_FOUND "-std=c++03")
			if(NOT FLAG_FOUND)
				check_compiler_flag(FLAG_FOUND "-std=c++98")
			endif()
		endif()
		if(NOT FLAG_FOUND)
			# Compiler does not support he -std flag, assume the highest supported C++ version is available
			# by default or can be enabled by CMake and rely on tests for individual features.
			foreach(ver IN LISTS versions)
				if(NOT version LESS 20${ver})
					set(CXX_VERSION 20${ver})
					break()
				endif()
			endforeach()
		endif()
		if(SET_WARNING_FLAGS AND NOT CXX_VERSION LESS 2011)
			add_cxxflag("-pedantic")
		endif()
	endif()
	
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}" PARENT_SCOPE)
	set(CXX_VERSION ${CXX_VERSION} PARENT_SCOPE)
	
	# Tell CMake about our desired C++ version so that it doesn't override our value with a lower version.
	# We check -std ourselves first because
	# - This feature is new in CMake 3.1
	# - Not all CMake versions know how to check for all C++ versions
	# - CMake doesn't tell us what versions are available
	if(NOT CMAKE_VERSION VERSION_LESS 3.12)
		set(max_cxx_standard 20)
	elseif(NOT CMAKE_VERSION VERSION_LESS 3.8)
		set(max_cxx_standard 17)
	else()
		set(max_cxx_standard 14)
	endif()
	foreach(ver IN LISTS versions)
		if(NOT CXX_VERSION LESS 20${ver} AND NOT max_cxx_standard LESS ver)
			set(CMAKE_CXX_STANDARD ${ver} PARENT_SCOPE)
			set(CMAKE_CXX_STANDARD_REQUIRED OFF PARENT_SCOPE)
			set(CMAKE_CXX_EXTENSIONS OFF PARENT_SCOPE)
			break()
		endif()
	endforeach()
	
endfunction(enable_cxx_version)

function(check_cxx version feature resultvar)
	set(result)
	if(NOT CXX_VERSION LESS 20${version} OR (ARGC GREATER 3 AND ARGV3 STREQUAL "ALWAYS"))
		if(MSVC AND ARGC GREATER 3)
			if(NOT MSVC_VERSION LESS ARGV3)
				set(result 1)
			endif()
		else()
			string(REGEX REPLACE "[^a-zA-Z0-9_][^a-zA-Z0-9_]*" "-" check "${feature}")
			string(REGEX REPLACE "^--*" "" check "${check}")
			string(REGEX REPLACE "--*$" "" check "${check}")
			set(file "${CXX_CHECK_DIR}/cxx${version}-${check}.cpp")
			set(old_CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
			set(old_CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS}")
			strip_warning_flags(CMAKE_CXX_FLAGS)
			strip_warning_flags(CMAKE_EXE_LINKER_FLAGS)
			check_compile(result "${file}" "${feature}" "C++${version} feature")
			set(CMAKE_CXX_FLAGS "${old_CMAKE_CXX_FLAGS}")
			set(CMAKE_EXE_LINKER_FLAGS "${old_CMAKE_EXE_LINKER_FLAGS}")
		endif()
	endif()
	if(NOT DEFINED result OR result STREQUAL "")
		set(${resultvar} OFF PARENT_SCOPE)
	else()
		set(${resultvar} ON PARENT_SCOPE)
	endif()
endfunction()

macro(check_cxx11 feature resultvar)
	check_cxx(11 ${ARGV})
endmacro()

macro(check_cxx14 feature resultvar)
	check_cxx(14 ${ARGV})
endmacro()

macro(check_cxx17 feature resultvar)
	check_cxx(17 ${ARGV})
endmacro()
