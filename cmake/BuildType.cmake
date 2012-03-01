
include(CompileCheck)

if(CMAKE_BUILD_TYPE STREQUAL "")
	set(CMAKE_BUILD_TYPE "Release")
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
	
	add_definitions(-DDEBUG)
	set(DEBUG 1)
	
	check_compiler_flag(RESULT "-g3")
	if(NOT RESULT STREQUAL "")
		string(REGEX REPLACE "-g(|[0-9]|gdb)" "" CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
		set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} ${RESULT}")
	endif()
	
	check_compiler_flag(RESULT "-O0")
	string(REGEX REPLACE "-O[0-9]" "" CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} ${RESULT}")
	
endif()
