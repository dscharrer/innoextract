
include(CompileCheck)

option(DEBUG_EXTRA "Expensive debug options" OFF)
option(SET_WARNING_FLAGS "Adjust compiler warning flags" ON)
option(SET_OPTIMIZATION_FLAGS "Adjust compiler optimization flags" ON)

if(SET_WARNING_FLAGS)
	
	# GCC (and compatible)
	add_cxxflag("-Wall")
	add_cxxflag("-Wextra")
	add_cxxflag("-Wformat=2")
	add_cxxflag("-Wundef")
	add_cxxflag("-Wpointer-arith")
	add_cxxflag("-Wcast-qual")
	add_cxxflag("-Woverloaded-virtual")
	add_cxxflag("-Wlogical-op")
	add_cxxflag("-Woverflow")
	add_cxxflag("-Wconversion")
	add_cxxflag("-Wsign-conversion")
	add_cxxflag("-Wmissing-declarations")
	add_cxxflag("-Wredundant-decls")
	
	# clang
	add_cxxflag("-Wliteral-conversion")
	add_cxxflag("-Wshift-overflow")
	add_cxxflag("-Wbool-conversions")
	
	# icc
	if(NOT DEBUG_EXTRA)
		add_cxxflag("-wd1418") # 'external function definition with no prior declaration'
	endif()
	
endif(SET_WARNING_FLAGS)

if(DEBUG_EXTRA)
	add_cxxflag("-ftrapv") # to add checks for (undefined) signed integer overflow
	add_cxxflag("-fbounds-checking")
	add_cxxflag("-fcatch-undefined-behavior")
	add_cxxflag("-Wstrict-aliasing=1")
endif(DEBUG_EXTRA)

if(CMAKE_BUILD_TYPE STREQUAL "")
	set(CMAKE_BUILD_TYPE "Release")
endif()
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
	add_definitions(-DDEBUG)
	set(DEBUG 1)
endif()

if(SET_OPTIMIZATION_FLAGS)
	
	# Link as few libraries as possible
	# This is much easier than trying to decide which libraries are needed for each system
	# Specifically, then need for libboost_system depends on the Boost version
	add_ldflag("-Wl,--as-needed")
	
	if(CMAKE_BUILD_TYPE STREQUAL "Debug")
		
		# set debug symbol level to -g3
		check_compiler_flag(RESULT "-g3")
		if(NOT RESULT STREQUAL "")
			string(REGEX REPLACE "-g(|[0-9]|gdb)" "" CMAKE_CXX_FLAGS_DEBUG
			       "${CMAKE_CXX_FLAGS_DEBUG}")
			set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} ${RESULT}")
		endif()
		
		# disable optimizations
		check_compiler_flag(RESULT "-O0")
		string(REGEX REPLACE "-O[0-9]" "" CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
		set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} ${RESULT}")
		
	endif()
	
endif(SET_OPTIMIZATION_FLAGS)
