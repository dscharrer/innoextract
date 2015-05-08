
include(CompileCheck)

option(DEBUG_EXTRA "Expensive debug options" OFF)
option(SET_WARNING_FLAGS "Adjust compiler warning flags" ON)
option(SET_OPTIMIZATION_FLAGS "Adjust compiler optimization flags" ON)

if(CMAKE_BUILD_TYPE STREQUAL "")
	set(CMAKE_BUILD_TYPE "Release")
endif()

if(MSVC)
	
	if(SET_WARNING_FLAGS)
		add_definitions(/wd4250) # harasses you when inheriting from std::basic_{i,o}stream
		add_definitions(/wd4996) # 'unsafe' stdlib functions used by Boost
	endif()
	
	if(SET_OPTIMIZATION_FLAGS)
		# Enable linker optimization in release
		#  /OPT:REF   Eliminate unreferenced code
		#  /OPT:ICF   COMDAT folding (merge functions generating the same code)
		#  /GL + /LTCG
		set(CMAKE_CXX_FLAGS_RELEASE
		    "${CMAKE_CXX_FLAGS_RELEASE} /Ox /Os /GL")
		if(CMAKE_SIZEOF_VOID_P EQUAL 4)
			set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /arch:SSE2")
		endif()
		set(CMAKE_EXE_LINKER_FLAGS_RELEASE
		    "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /OPT:REF /OPT:ICF /LTCG")
		set(CMAKE_SHARED_LINKER_FLAGS_RELEASE
		    "${CMAKE_SHARED_LINKER_FLAGS_RELEASE} /OPT:REF /OPT:ICF /LTCG")
	endif()
	
else(MSVC)
	
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
		if(NOT DEBUG_EXTRA AND CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
			# GCC is 'clever' and silently accepts -Wno-*  - check for the non-negated variant
			check_compiler_flag(FLAG_FOUND "-Wmaybe-uninitialized")
			if(FLAG_FOUND)
				add_cxxflag("-Wno-maybe-uninitialized")
			endif()
		endif()
		
		# clang
		add_cxxflag("-Wliteral-conversion")
		add_cxxflag("-Wshift-overflow")
		add_cxxflag("-Wbool-conversions")
		add_cxxflag("-Wheader-guard")
		add_cxxflag("-Wpessimizing-move")
		
		# icc
		if(NOT DEBUG_EXTRA AND CMAKE_CXX_COMPILER_ID STREQUAL "Intel")
			# 'external function definition with no prior declaration'
			# This gets annoying fast with small inline/template functions.
			add_cxxflag("-wd1418")
		endif()
		
		# EKOPath
		if(NOT DEBUG_EXTRA AND CMAKE_CXX_COMPILER_ID STREQUAL "PathScale")
			# This triggers on every use of BOOST_STATIC_ASSERT
			add_cxxflag("-Wno-unused-local-typedef")
		endif()
		
	endif(SET_WARNING_FLAGS)
	
	if(DEBUG_EXTRA)
		add_cxxflag("-ftrapv") # to add checks for (undefined) signed integer overflow
		add_cxxflag("-fbounds-checking")
		add_cxxflag("-fcatch-undefined-behavior")
		add_cxxflag("-Wstrict-aliasing=1")
	endif(DEBUG_EXTRA)
	
	if(SET_OPTIMIZATION_FLAGS)
		
		# Link as few libraries as possible
		# This is much easier than trying to decide which libraries are needed for each system
		# Specifically, the need for libboost_system depends on the Boost version
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
	
endif(MSVC)
