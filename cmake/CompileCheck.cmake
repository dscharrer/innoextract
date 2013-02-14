
# Copyright (C) 2011-2012 Daniel Scharrer
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

function(check_flag RESULT FLAG TYPE)
	
	if(DEFINED CHECK_COMPILER_FLAG_${FLAG})
		if(CHECK_COMPILER_FLAG_${FLAG})
			set(${RESULT} "${FLAG}" PARENT_SCOPE)
		else()
			set(${RESULT} "" PARENT_SCOPE)
		endif()
		return()
	endif()
	
	# CMake already has a check_cxx_compiler_flag macro in CheckCXXCompilerFlag, but
	# it prints the result variable in the output (which is ugly!) and also uses it
	# as a key to cache checks - so it would need to be unique for each flag.
	# Unfortunately it also naively pastes the variable name inside a regexp so
	# if we tried to use the flag itself in the variable name it will fail for -std=c++11.
	# But we can at least use the expressions for warnings from that macro (and more):
	set(fail_regexps
		"warning:"                                     # general
		"unrecognized .*option"                        # GNU
		"unknown .*option"                             # Clang
		"ignoring unknown option"                      # MSVC
		"warning D9002"                                # MSVC, any lang
		"option.*not supported"                        # Intel
		"invalid argument .*option"                    # Intel
		"ignoring option .*argument required"          # Intel
		"command line warning"                         # Intel
		"[Uu]nknown option"                            # HP
		"[Ww]arning: [Oo]ption"                        # SunPro
		"command option .* is not recognized"          # XL
		"not supported in this configuration; ignored" # AIX
		"File with unknown suffix passed to linker"    # PGI
		"WARNING: unknown flag:"                       # Open64
	)
	
	# Set the flags to check
	set(old_CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
	set(old_CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS}")
	set(old_CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS}")
	set(old_CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS}")
	if("${TYPE}" STREQUAL linker)
		set(CMAKE_EXE_LINKER_FLAGS "${old_CMAKE_EXE_LINKER_FLAGS} ${FLAG}")
		set(CMAKE_SHARED_LINKER_FLAGS "${old_CMAKE_SHARED_LINKER_FLAGS} ${FLAG}")
		set(CMAKE_MODULE_LINKER_FLAGS "${old_CMAKE_MODULE_LINKER_FLAGS} ${FLAG}")
	else()
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${FLAG}")
	endif()
	
	# Check if we can compile and link a simple file with the new flags
	set(compile_test_file "${CMAKE_CURRENT_BINARY_DIR}/compile_flag_test.cpp")
	file(WRITE ${compile_test_file} "__attribute__((const)) int main(){ return 0; }\n")
	try_compile(
		CHECK_COMPILER_FLAG ${CMAKE_BINARY_DIR} ${compile_test_file}
		CMAKE_FLAGS "-DCMAKE_CXX_FLAGS=${CMAKE_CXX_FLAGS}"
		            "-DCMAKE_EXE_LINKER_FLAGS=${CMAKE_EXE_LINKER_FLAGS}"
		            "-DCMAKE_SHARED_LINKER_FLAGS=${CMAKE_SHARED_LINKER_FLAGS}"
		            "-DCMAKE_MODULE_LINKER_FLAGS=${CMAKE_MODULE_LINKER_FLAGS}"
		OUTPUT_VARIABLE ERRORLOG
	)
	
	# Restore the old flags
	set(CMAKE_CXX_FLAGS "${old_CMAKE_CXX_FLAGS}")
	set(CMAKE_EXE_LINKER_FLAGS "${old_CMAKE_EXE_LINKER_FLAGS}")
	set(CMAKE_SHARED_LINKER_FLAGS "${old_CMAKE_SHARED_LINKER_FLAGS}")
	set(CMAKE_MODULE_LINKER_FLAGS "${old_CMAKE_MODULE_LINKER_FLAGS}")
	
	if(NOT CHECK_COMPILER_FLAG)
		message(STATUS "Checking ${TYPE} flag: ${FLAG} - unsupported")
		set(${RESULT} "" PARENT_SCOPE)
		set("CHECK_COMPILER_FLAG_${FLAG}" 0 CACHE INTERNAL "...")
	else()
		
		set(has_warning 0)
		foreach(expr IN LISTS fail_regexps)
			if("${ERRORLOG}" MATCHES "${expr}")
				set(has_warning 1)
			endif()
		endforeach()
		
		if(has_warning)
			message(STATUS "Checking ${TYPE} flag: ${FLAG} - unsupported (warning)")
			set(${RESULT} "" PARENT_SCOPE)
			set("CHECK_COMPILER_FLAG_${FLAG}" 0 CACHE INTERNAL "...")
		else()
			message(STATUS "Checking ${TYPE} flag: ${FLAG}")
			set(${RESULT} "${FLAG}" PARENT_SCOPE)
			set("CHECK_COMPILER_FLAG_${FLAG}" 1 CACHE INTERNAL "...")
		endif()
		
	endif()
	
endfunction(check_flag)

function(check_compiler_flag RESULT FLAG)
	check_flag(result "${FLAG}" compiler)
	set(${RESULT} "${result}" PARENT_SCOPE)
endfunction(check_compiler_flag)

function(check_linker_flag RESULT FLAG)
	check_flag(result "${FLAG}" linker)
	set(${RESULT} "${result}" PARENT_SCOPE)
endfunction(check_linker_flag)

function(add_cxxflag FLAG)
	
	check_compiler_flag(RESULT "${FLAG}")
	
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${RESULT}" PARENT_SCOPE)
	
	if(RESULT STREQUAL "")
		set(FLAG_FOUND 0 PARENT_SCOPE)
	else()
		set(FLAG_FOUND 1 PARENT_SCOPE)
	endif()
	
endfunction(add_cxxflag)

function(add_ldflag FLAG)
	
	check_linker_flag(RESULT "${FLAG}")
	
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${RESULT}" PARENT_SCOPE)
	set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} ${RESULT}" PARENT_SCOPE)
	set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} ${RESULT}" PARENT_SCOPE)
	
	if(RESULT STREQUAL "")
		set(FLAG_FOUND 0 PARENT_SCOPE)
	else()
		set(FLAG_FOUND 1 PARENT_SCOPE)
	endif()
	
endfunction(add_ldflag)

function(try_link_library LIBRARY_NAME LIBRARY_FILE ERROR_VAR)
	# See if we can link a simple program with the library using the configured c++ compiler
	set(link_test_file "${CMAKE_CURRENT_BINARY_DIR}/link_test.cpp")
	file(WRITE ${link_test_file} "int main(){}\n")
	if(CMAKE_THREAD_LIBS_INIT)
		list(APPEND LIBRARY_FILE "${CMAKE_THREAD_LIBS_INIT}")
	endif()
	try_compile(
		CHECK_${LIBRARY_NAME}_LINK "${CMAKE_BINARY_DIR}" "${link_test_file}"
		CMAKE_FLAGS "-DLINK_LIBRARIES=${LIBRARY_FILE}"
		            "-DCMAKE_CXX_FLAGS=${CMAKE_CXX_FLAGS}"
		            "-DCMAKE_EXE_LINKER_FLAGS=${CMAKE_EXE_LINKER_FLAGS}"
		            "-DCMAKE_SHARED_LINKER_FLAGS=${CMAKE_SHARED_LINKER_FLAGS}"
		            "-DCMAKE_MODULE_LINKER_FLAGS=${CMAKE_MODULE_LINKER_FLAGS}"
		OUTPUT_VARIABLE ERRORLOG
	)
	set(${ERROR_VAR} "${ERRORLOG}" PARENT_SCOPE)
endfunction(try_link_library)

##############################################################################
# Check that a a library actually works for the current configuration
function(check_link_library LIBRARY_NAME LIBRARY_VARIABLE)
	
	set(lib_current "${${LIBRARY_VARIABLE}}")
	set(found_var "ARX_CLL_${LIBRARY_NAME}_FOUND")
	set(working_var "ARX_CLL_${LIBRARY_NAME}_WORKING")
	
	if(CHECK_${LIBRARY_NAME}_LINK)
		set(lib_found "${${found_var}}")
		set(lib_working "${${working_var}}")
		if((lib_current STREQUAL lib_found) OR (lib_current STREQUAL lib_working))
			set("${LIBRARY_VARIABLE}" "${lib_working}" PARENT_SCOPE)
			return()
		endif()
	endif()
	
	set("${found_var}" "${lib_current}" CACHE INTERNAL "...")
	
	if(NOT lib_current STREQUAL "")
		message(STATUS "Checking ${LIBRARY_NAME}: ${lib_current}")
	endif()
	
	# Check if we can link to the full path found by find_package
	try_link_library(${LIBRARY_NAME} "${lib_current}" ERRORLOG1)
	
	if(CHECK_${LIBRARY_NAME}_LINK)
		set("${working_var}" "${lib_current}" CACHE INTERNAL "...")
		return()
	endif()
	
	# Check if the linker is smarter than cmake and try to link with only the library name
	string(REGEX REPLACE "(^|;)[^;]*/lib([^;/]*)\\.so" "\\1-l\\2"
	       LIBRARY_FILE "${lib_current}")
	
	if(NOT LIBRARY_FILE STREQUAL lib_current)
		
		try_link_library(${LIBRARY_NAME} "${LIBRARY_FILE}" ERRORLOG2)
		
		if(CHECK_${LIBRARY_NAME}_LINK)
			message(STATUS " -> using ${LIBRARY_FILE} instead")
			set("${LIBRARY_VARIABLE}" "${LIBRARY_FILE}" PARENT_SCOPE)
			set("${working_var}" "${LIBRARY_FILE}" CACHE INTERNAL "...")
			return()
		endif()
		
	endif()
	
	# Force cmake to search again, as the cached library doesn't work
	unset(FIND_PACKAGE_MESSAGE_DETAILS_${ARGV2} CACHE)
	unset(FIND_PACKAGE_MESSAGE_DETAILS_${LIBRARY_NAME} CACHE)
	
	message(FATAL_ERROR "\n${ERRORLOG1}\n\n${ERRORLOG2}\n\n"
	        "!! No suitable version of ${LIBRARY_NAME} found.\n"
	        "   Maybe you don't have the right (32 vs.64 bit) architecture installed?\n\n"
	        "   Tried ${lib_current} and ${LIBRARY_FILE}\n"
	        "   Using compiler ${CMAKE_CXX_COMPILER} ${CMAKE_CXX_FLAGS}\n\n\n")
	
endfunction(check_link_library)

function(force_recheck_library LIBRARY_NAME)
	unset(FIND_PACKAGE_MESSAGE_DETAILS_${ARGV1} CACHE)
	unset(FIND_PACKAGE_MESSAGE_DETAILS_${LIBRARY_NAME} CACHE)
	unset(CHECK_${LIBRARY_NAME}_LINK CACHE)
endfunction()
