
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

# Note: In CMake before 3.0 set(var "" PARENT_SCOPE) *unsets* the variable in the
# parent scope instead of setting it to the empty string.
# This means if(var STREQUAL "") will be false since var is not defined and thus not expanded.

function(check_compile RESULT FILE FLAG TYPE)
	
	string(REGEX REPLACE "[^a-zA-Z0-9_][^a-zA-Z0-9_]*" "-" cachevar "${TYPE}-${FLAG}")
	set(cahevar "CHECK_COMPILE_${cahevar}")
	
	if(DEFINED ${cachevar})
		if(${cachevar})
			set(${RESULT} "${FLAG}" PARENT_SCOPE)
		else()
			set(${RESULT} "" PARENT_SCOPE)
		endif()
		return()
	endif()
	
	string(REGEX REPLACE "[^a-zA-Z0-9]" "\\\\\\0" escaped_flag ${FLAG} )
	
	# CMake already has a check_cxx_compiler_flag macro in CheckCXXCompilerFlag, but
	# it prints the result variable in the output (which is ugly!) and also uses it
	# as a key to cache checks - so it would need to be unique for each flag.
	# Unfortunately it also naively pastes the variable name inside a regexp so
	# if we tried to use the flag itself in the variable name it will fail for -std=c++11.
	# But we can at least use the expressions for warnings from that macro (and more):
	set(fail_regexps
		"warning:"                                     # general
		"unrecognized .*option"                        # GNU
		"${escaped_flag}.* not supported"              # GNU
		"unknown .*option"                             # Clang
		"ignoring unknown option"                      # MSVC
		"warning D9002"                                # MSVC, any lang
		"warning #[0-9]*:"                             # Intel
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
	if(TYPE STREQUAL "linker flag")
		set(CMAKE_EXE_LINKER_FLAGS "${old_CMAKE_EXE_LINKER_FLAGS} ${FLAG}")
		set(CMAKE_SHARED_LINKER_FLAGS "${old_CMAKE_SHARED_LINKER_FLAGS} ${FLAG}")
		set(CMAKE_MODULE_LINKER_FLAGS "${old_CMAKE_MODULE_LINKER_FLAGS} ${FLAG}")
	elseif(TYPE STREQUAL "compiler flag")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${FLAG}")
	endif()
	
	# Check if we can compile and link a simple file with the new flags
	try_compile(
		check_compiler_flag ${PROJECT_BINARY_DIR} ${FILE}
		CMAKE_FLAGS "-DCMAKE_CXX_FLAGS:STRING=${CMAKE_CXX_FLAGS}"
		            "-DCMAKE_EXE_LINKER_FLAGS:STRING=${CMAKE_EXE_LINKER_FLAGS}"
		            "-DCMAKE_SHARED_LINKER_FLAGS:STRING=${CMAKE_SHARED_LINKER_FLAGS}"
		            "-DCMAKE_MODULE_LINKER_FLAGS:STRING=${CMAKE_MODULE_LINKER_FLAGS}"
		OUTPUT_VARIABLE ERRORLOG
	)
	
	# Restore the old flags
	set(CMAKE_CXX_FLAGS "${old_CMAKE_CXX_FLAGS}")
	set(CMAKE_EXE_LINKER_FLAGS "${old_CMAKE_EXE_LINKER_FLAGS}")
	set(CMAKE_SHARED_LINKER_FLAGS "${old_CMAKE_SHARED_LINKER_FLAGS}")
	set(CMAKE_MODULE_LINKER_FLAGS "${old_CMAKE_MODULE_LINKER_FLAGS}")
	
	if(NOT check_compiler_flag)
		message(STATUS "Checking ${TYPE}: ${FLAG} - unsupported")
		set(${RESULT} "" PARENT_SCOPE)
		set("${cachevar}" 0 CACHE INTERNAL "...")
	else()
		
		set(has_warning 0)
		foreach(expr IN LISTS fail_regexps)
			if("${ERRORLOG}" MATCHES "${expr}")
				set(has_warning 1)
			endif()
		endforeach()
		
		if(has_warning)
			message(STATUS "Checking ${TYPE}: ${FLAG} - unsupported (warning)")
			set(${RESULT} "" PARENT_SCOPE)
			set("${cachevar}" 0 CACHE INTERNAL "...")
		else()
			message(STATUS "Checking ${TYPE}: ${FLAG}")
			set(${RESULT} "${FLAG}" PARENT_SCOPE)
			set("${cachevar}" 1 CACHE INTERNAL "...")
		endif()
		
	endif()
	
endfunction(check_compile)

function(check_flag RESULT FLAG TYPE)
	set(compile_test_file "${CMAKE_CURRENT_BINARY_DIR}/compile_flag_test.cpp")
	if(MSVC)
		file(WRITE ${compile_test_file} "int main(){ return 0; }\n")
	else()
		file(WRITE ${compile_test_file} "__attribute__((const)) int main(){ return 0; }\n")
	endif()
	check_compile(result "${compile_test_file}" "${FLAG}" "${TYPE} flag")
	set(${RESULT} "${result}" PARENT_SCOPE)
endfunction(check_flag)

macro(strip_warning_flags VAR)
	string(REGEX REPLACE "(^| )\\-(W[^ ]*|pedantic)" "" ${VAR} "${${VAR}}")
endmacro()

function(check_builtin RESULT EXPR)
	string(REGEX REPLACE "[^a-zA-Z0-9_][^a-zA-Z0-9_]*" "-" check "${EXPR}")
	string(REGEX REPLACE "_*\\-_*" "-" check "${check}")
	string(REGEX REPLACE "^[_\\-]+" "" check "${check}")
	string(REGEX REPLACE "[_\\-]+$" "" check "${check}")
	set(compile_test_file "${CMAKE_CURRENT_BINARY_DIR}/check-builtin-${check}.cpp")
	string(REGEX MATCH "[a-zA-Z_][a-zA-Z_0-9]*" type "${EXPR}")
	file(WRITE ${compile_test_file} "__attribute__((const)) int main(){ (void)(${EXPR}); return 0; }\n")
	set(old_CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
	set(old_CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS}")
	strip_warning_flags(CMAKE_CXX_FLAGS)
	strip_warning_flags(CMAKE_EXE_LINKER_FLAGS)
	check_compile(result "${compile_test_file}" "${type}" "compiler builtin")
	set(CMAKE_CXX_FLAGS "${old_CMAKE_CXX_FLAGS}")
	set(CMAKE_EXE_LINKER_FLAGS "${old_CMAKE_EXE_LINKER_FLAGS}")
	set(${RESULT} "${result}" PARENT_SCOPE)
endfunction(check_builtin)

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
	
	if(NOT DEFINED RESULT OR RESULT STREQUAL "")
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
	
	if(NOT DEFINED RESULT OR RESULT STREQUAL "")
		set(FLAG_FOUND 0 PARENT_SCOPE)
	else()
		set(FLAG_FOUND 1 PARENT_SCOPE)
	endif()
	
endfunction(add_ldflag)
