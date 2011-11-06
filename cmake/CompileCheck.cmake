
function(check_compiler_flag RESULT FLAG)
	
	if(DEFINED CHECK_COMPILER_FLAG_${FLAG})
		if(CHECK_COMPILER_FLAG_${FLAG})
			set(${RESULT} "${FLAG}" PARENT_SCOPE)
		else()
			set(${RESULT} "" PARENT_SCOPE)
		endif()
		return()
	endif()
	
	set(compile_test_file "${CMAKE_CURRENT_BINARY_DIR}/compile_flag_test.cpp")
	file(WRITE ${compile_test_file} "__attribute__((const)) int main(){ return 0; }\n")
	try_compile(CHECK_COMPILER_FLAG ${CMAKE_BINARY_DIR} ${compile_test_file} COMPILE_DEFINITIONS "${FLAG}" OUTPUT_VARIABLE ERRORLOG)
	
	string(REGEX MATCH "warning:" HAS_WARNING "${ERRORLOG}")
	
	if(NOT CHECK_COMPILER_FLAG)
		message(STATUS "Checking compiler flag: ${FLAG} - unsupported")
		set(${RESULT} "" PARENT_SCOPE)
		set("CHECK_COMPILER_FLAG_${FLAG}" 0 CACHE INTERNAL "...")
	elseif(NOT HAS_WARNING STREQUAL "")
		message(STATUS "Checking compiler flag: ${FLAG} - unsupported (warning)")
		set(${RESULT} "" PARENT_SCOPE)
		set("CHECK_COMPILER_FLAG_${FLAG}" 0 CACHE INTERNAL "...")
	else()
		message(STATUS "Checking compiler flag: ${FLAG}")
		set(${RESULT} "${FLAG}" PARENT_SCOPE)
		set("CHECK_COMPILER_FLAG_${FLAG}" 1 CACHE INTERNAL "...")
	endif()
	
endfunction(check_compiler_flag)

function(add_cxxflag FLAG)
	
	check_compiler_flag(RESULT "${FLAG}")
	
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${RESULT}" PARENT_SCOPE)
	
endfunction(add_cxxflag)

function(try_link_library LIBRARY_NAME LIBRARY_FILE ERROR_VAR)
	# See if we can link a simple program with the library using the configured c++ compiler.
	set(link_test_file "${CMAKE_CURRENT_BINARY_DIR}/link_test.cpp")
	file(WRITE ${link_test_file} "int main(){}\n")
	if(CMAKE_THREAD_LIBS_INIT)
		list(APPEND LIBRARY_FILE "${CMAKE_THREAD_LIBS_INIT}")
	endif()
	try_compile(CHECK_${LIBRARY_NAME}_LINK "${CMAKE_BINARY_DIR}" "${link_test_file}" CMAKE_FLAGS "-DLINK_LIBRARIES=${LIBRARY_FILE}" OUTPUT_VARIABLE ERRORLOG)
	set(${ERROR_VAR} "${ERRORLOG}" PARENT_SCOPE)
endfunction(try_link_library)

##############################################################################
# Check that a a library actually works for the current configuration.
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
	
	message(STATUS "Checking ${LIBRARY_NAME}: ${lib_current}")
	
	# Check if we can link to the full path found by find_package.
	try_link_library(${LIBRARY_NAME} "${lib_current}" ERRORLOG1)
	
	if(CHECK_${LIBRARY_NAME}_LINK)
		set("${working_var}" "${lib_current}" CACHE INTERNAL "...")
		return()
	endif()
	
	# Check if the linker is smarter than cmake and try to link with only the library name.
	string(REGEX REPLACE "(^|;)[^;]*/lib([^;/]*)\\.so" "\\1-l\\2" LIBRARY_FILE "${lib_current}")
	try_link_library(${LIBRARY_NAME} "${LIBRARY_FILE}" ERRORLOG2)
	
	if(CHECK_${LIBRARY_NAME}_LINK)
		message(STATUS " -> using ${LIBRARY_FILE} instead")
		set("${LIBRARY_VARIABLE}" "${LIBRARY_FILE}" PARENT_SCOPE)
		set("${working_var}" "${LIBRARY_FILE}" CACHE INTERNAL "...")
		return()
	endif()
	
	# Force cmake to search again, as the cached library doesn't work.
	unset(FIND_PACKAGE_MESSAGE_DETAILS_${ARGV2} CACHE)
	unset(FIND_PACKAGE_MESSAGE_DETAILS_${LIBRARY_NAME} CACHE)
	
	message(FATAL_ERROR "\n${ERRORLOG1}\n\n${ERRORLOG2}\n\n!! No suitable (32- vs. 64-bit) version of ${LIBRARY_NAME} found; tried ${lib_current} and ${LIBRARY_FILE}\nusing compiler ${CMAKE_CXX_COMPILER} ${CMAKE_CXX_FLAGS}\n")
	
endfunction(check_link_library)

function(force_recheck_library LIBRARY_NAME)
	unset(FIND_PACKAGE_MESSAGE_DETAILS_${ARGV1} CACHE)
	unset(FIND_PACKAGE_MESSAGE_DETAILS_${LIBRARY_NAME} CACHE)
	unset(CHECK_${LIBRARY_NAME}_LINK CACHE)
endfunction()
