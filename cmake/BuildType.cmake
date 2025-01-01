
include(CompileCheck)

if(NOT MSVC)
	include(CheckCXXSymbolExists)
	check_cxx_symbol_exists(_LIBCPP_VERSION "cstddef" IS_LIBCXX)
	if(IS_LIBCXX AND (DEBUG OR DEBUG_EXTRA))
		check_cxx_symbol_exists(_LIBCPP_HARDENING_MODE "version" ARX_HAVE_LIBCPP_HARDENING_MODE)
		if(NOT ARX_HAVE_LIBCPP_HARDENING_MODE)
			check_cxx_symbol_exists(_LIBCPP_ENABLE_HARDENED_MODE "version" ARX_HAVE_LIBCPP_ENABLE_HARDENED_MODE)
			if(NOT ARX_HAVE_LIBCPP_ENABLE_HARDENED_MODE)
				check_cxx_symbol_exists(_LIBCPP_ENABLE_ASSERTIONS "version" ARX_HAVE_LIBCPP_ENABLE_ASSERTIONS)
			endif()
		endif()
	endif()
else()
	set(IS_LIBCXX OFF)
endif()


option(DEBUG_EXTRA "Expensive debug options" OFF)
option(SET_WARNING_FLAGS "Adjust compiler warning flags" ON)
option(SET_NOISY_WARNING_FLAGS "Enable noisy compiler warnings" OFF)
option(SET_OPTIMIZATION_FLAGS "Adjust compiler optimization flags" ON)

if(MSVC)
	
	if(USE_LTO)
		set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /GL")
		set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /LTCG")
		set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "${CMAKE_SHARED_LINKER_FLAGS_RELEASE} /LTCG")
		set(CMAKE_STATIC_LINKER_FLAGS_RELEASE "${CMAKE_STATIC_LINKER_FLAGS_RELEASE} /LTCG")
	endif()
	
	if(FASTLINK)
		
		# Optimize for link speed in developer builds
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /DEBUG:FASTLINK")
		
	elseif(SET_OPTIMIZATION_FLAGS)
		
		# Merge symbols and discard unused symbols
		set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /OPT:REF /OPT:ICF")
		set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "${CMAKE_SHARED_LINKER_FLAGS_RELEASE} /OPT:REF /OPT:ICF")
		
	endif()
	
	if(SET_WARNING_FLAGS AND NOT SET_NOISY_WARNING_FLAGS)
		
		# TODO TEMP - disable very noisy warning
		# Conversion from 'A' to 'B', possible loss of data
		add_definitions(/wd4244)
		# warning C4245: 'return': conversion from 'A' to 'B', signed/unsigned mismatch
		add_definitions(/wd4245)
		# warning C4456: declaration of 'xxx' hides previous local declaration
		add_definitions(/wd4456) # triggers on nested BOOST_FOREACH
		# warning C4457: declaration of 'xxx' hides function parameter
		add_definitions(/wd4457)
		# warning C4458: declaration of 'xxx' hides class member
		add_definitions(/wd4458)
		# warning C4459: declaration of 'xxx' hides global declaration
		add_definitions(/wd4459)
		
		# warning C4127: conditional expression is constant
		add_definitions(/wd4127)
		if(MSVC_VERSION LESS 1900)
			# warning C4250: 'xxx': inherits 'std::basic_{i,o}stream::...' via dominance
			add_definitions(/wd4250) # harasses you when inheriting from std::basic_{i,o}stream
			# warning C4510: 'enum_names<const char *>' : default constructor could not be generated
			add_definitions(/wd4510)
			# warning C4512: 'xxx' : assignment operator could not be generated
			add_definitions(/wd4512) # not all classes need an assignment operator...
			# warning C4610: struct 'xxx' can never be instantiated - user defined constructor required
			add_definitions(/wd4610)
		endif()
		add_definitions(/wd4702) # warns in Boost
		add_definitions(/wd4706) # warns in Boost
		add_definitions(/wd4996) # 'unsafe' stdlib functions used by Boost
		
	endif()
	
	if(WERROR)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /WX")
	endif()
	
	if(SET_OPTIMIZATION_FLAGS)
		
		# Enable exceptions
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHsc")
		
		# Enable linker optimization in release
		set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /Ox /Oi /Os")
		
	endif()
	
	foreach(flag_var CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE)
		
		# Disable Run time checks
		if(NOT DEBUG_EXTRA)
			string(REGEX REPLACE "(^| )/RTC1( |$)" "\\1" ${flag_var} "${${flag_var}}")
		endif()
		
		# Remove definition of _DEBUG as it might conflict with libs we're linking with
		string(REGEX REPLACE "(^| )/D_DEBUG( |$)" "\\1" ${flag_var} "${${flag_var}}")
		set(${flag_var} "${${flag_var}} /DNDEBUG")
		
		# Force compiler warning level
		if(SET_WARNING_FLAGS)
			string(REGEX REPLACE "(^| )/W[0-4]( |$)" "\\1" ${flag_var} "${${flag_var}}")
			set(${flag_var} "${${flag_var}} /W4")
		endif()
		
	endforeach(flag_var)
	
	if(NOT MSVC_VERSION LESS 1900)
		add_definitions(/utf-8)
	endif()
	
	# Turn on standards compliant mode
	# /Za is not compatible with /fp:fast, leave it off
	if(NOT MSVC_VERSION LESS 1910)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /permissive-")
		# /permissive- enables /Zc:twoPhase wich would be good if two phase lookup wasn't still broken in VS 2017
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zc:twoPhase-")
	endif()
	if(NOT MSVC_VERSION LESS 1900)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zc:inline")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zc:throwingNew")
	endif()
	if(NOT MSVC_VERSION LESS 1914)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zc:__cplusplus")
	endif()
	
	# Always build with debug information
	if(NOT MSVC_VERSION LESS 1700)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zi")
		set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /DEBUG")
	endif()
	
else(MSVC)
	
	set(linker_used)
	if(NOT linker_used AND (USE_LD STREQUAL "mold" OR USE_LD STREQUAL "best"))
		# Old versions are unstable or don't support LTO
		if(USE_LD STREQUAL "best")
			execute_process(COMMAND ${CMAKE_CXX_COMPILER} "-fuse-ld=mold" "-Wl,-version"
			                OUTPUT_VARIABLE _Mold_Version ERROR_QUIET)
		endif()
		if(USE_LD STREQUAL "best" AND _Mold_Version MATCHES "mold [0-1]\\.*")
			message(STATUS "Not using ancient ${CMAKE_MATCH_0}")
		else()
			add_ldflag("-fuse-ld=mold")
			if(FLAG_FOUND)
				set(linker_used "mold")
			elseif(STRICT_USE AND NOT USE_LD STREQUAL "best")
				message(FATAL_ERROR "Requested linker is not available")
			endif()
		endif()
	endif()
	if(NOT linker_used AND (USE_LD STREQUAL "lld" OR
	                        (USE_LD STREQUAL "best" AND (NOT USE_LTO OR CMAKE_CXX_COMPILER_ID MATCHES "Clang"))))
		# Only supports LTO with LLVM-based compilers and old versions are unstable
		if(USE_LD STREQUAL "best")
			execute_process(COMMAND ${CMAKE_CXX_COMPILER} "-fuse-ld=lld" "-Wl,-version"
			                OUTPUT_VARIABLE _LLD_Version ERROR_QUIET)
		endif()
		if(USE_LD STREQUAL "best" AND _LLD_Version MATCHES "LLD [0-8]\\.[0-9\\.]*")
			message(STATUS "Not using ancient ${CMAKE_MATCH_0}")
		elseif(USE_LD STREQUAL "best" AND CMAKE_CXX_COMPILER_ID MATCHES "Clang" AND
		       CMAKE_CXX_COMPILER_VERSION VERSION_LESS 9)
			message(STATUS "Not using ancient Clang ${CMAKE_CXX_COMPILER_VERSION}")
		else()
			add_ldflag("-fuse-ld=lld")
			if(FLAG_FOUND)
				set(linker_used "lld")
			elseif(STRICT_USE AND NOT USE_LD STREQUAL "best")
				message(FATAL_ERROR "Requested linker is not available")
			endif()
		endif()
	endif()
	if(NOT linker_used AND (USE_LD STREQUAL "gold" OR USE_LD STREQUAL "best"))
		add_ldflag("-fuse-ld=gold")
		if(FLAG_FOUND)
			set(linker_used "gold")
		elseif(STRICT_USE AND NOT USE_LD STREQUAL "best")
			message(FATAL_ERROR "Requested linker is not available")
		endif()
	endif()
	if(NOT linker_used AND (USE_LD STREQUAL "bfd"))
		add_ldflag("-fuse-ld=bfd")
		if(FLAG_FOUND)
			set(linker_used "bfd")
		elseif(STRICT_USE AND NOT USE_LD STREQUAL "best")
			message(FATAL_ERROR "Requested linker is not available")
		endif()
	endif()
	
	if(USE_LTO)
		add_cxxflag("-flto=auto")
		if(NOT FLAG_FOUND)
			add_cxxflag("-flto")
		endif()
		# TODO set CMAKE_INTERPROCEDURAL_OPTIMIZATION instead
		add_ldflag("-fuse-linker-plugin")
	endif()
	
	if(FASTLINK)
		
		# Optimize for link speed in developer builds
		if(linker_used STREQUAL "mold" OR linker_used STREQUAL "lld")
			# mold and lld are fast enough without -gsplit-dwarf that we don't need to deal with its issues
		else()
			add_cxxflag("-gsplit-dwarf")
			add_cxxflag("-gdwarf-4") # -gsplit-dwarf is broken with DWARF 5
		endif()
		
	elseif(SET_OPTIMIZATION_FLAGS)
		
		# Merge symbols and discard unused symbols
		add_ldflag("-Wl,--gc-sections")
		add_ldflag("-Wl,--icf=all")
		add_cxxflag("-fmerge-all-constants")
		
	endif()
	
	if(SET_WARNING_FLAGS)
		
		# GCC or Clang (and compatible)
		
		add_cxxflag("-Wall")
		add_cxxflag("-Wextra")
		
		add_cxxflag("-Warray-bounds=2")
		add_cxxflag("-Wbool-conversions")
		add_cxxflag("-Wcast-qual")
		add_cxxflag("-Wcatch-value=3")
		add_cxxflag("-Wconversion")
		add_cxxflag("-Wdocumentation")
		add_cxxflag("-Wdouble-promotion")
		add_cxxflag("-Wduplicated-cond")
		add_cxxflag("-Wextra-semi")
		add_cxxflag("-Wformat=2")
		add_cxxflag("-Wheader-guard")
		add_cxxflag("-Winit-self")
		add_cxxflag("-Wkeyword-macro")
		add_cxxflag("-Wliteral-conversion")
		add_cxxflag("-Wlogical-op")
		add_cxxflag("-Wmissing-declarations")
		add_cxxflag("-Wnoexcept")
		add_cxxflag("-Woverflow")
		add_cxxflag("-Woverloaded-virtual")
		add_cxxflag("-Wpessimizing-move")
		add_cxxflag("-Wpointer-arith")
		add_cxxflag("-Wredundant-decls")
		add_cxxflag("-Wreserved-id-macro")
		add_cxxflag("-Wshift-overflow")
		add_cxxflag("-Wsign-conversion")
		add_cxxflag("-Wstrict-null-sentinel")
		add_cxxflag("-Wstringop-overflow=4")
		add_cxxflag("-Wundef")
		add_cxxflag("-Wunused-const-variable=1")
		add_cxxflag("-Wunused-macros")
		add_cxxflag("-Wvla")
		
		if(NOT CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS 4.8)
			add_cxxflag("-Wold-style-cast")
		endif()
		
		if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 5
		   AND NOT SET_NOISY_WARNING_FLAGS)
			# In older GCC versions this warning is too strict
		elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 5
		       AND NOT SET_NOISY_WARNING_FLAGS)
			# In older Clang verstions this warns on BOOST_SCOPE_EXIT
		elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Intel" AND NOT SET_NOISY_WARNING_FLAGS)
			# For icc this warning is too strict
		else()
			add_cxxflag("-Wshadow")
		endif()
		
		add_ldflag("-Wl,--no-undefined")
		
		if(SET_NOISY_WARNING_FLAGS)
			
			# These are too noisy to enable right now but we still want to track new warnings.
			# TODO enable by default as soon as most are silenced
			add_cxxflag("-Wconditionally-supported") # warns on casting from pointer to function pointer
			add_cxxflag("-Wduplicated-branches")
			add_cxxflag("-Wstrict-aliasing=1") # has false positives
			add_cxxflag("-Wuseless-cast") # has false positives
			add_cxxflag("-Wsign-promo")
			# add_cxxflag("-Wnull-dereference") not that useful without deduction path
			
			# Possible optimization opportunities
			add_cxxflag("-Wdisabled-optimization")
			add_cxxflag("-Wpadded")
			add_cxxflag("-Wunsafe-loop-optimizations")
			
			if(NOT DEBUG_EXTRA OR NOT CMAKE_CXX_COMPILER_ID MATCHES "Clang")
				add_ldflag("-Wl,--detect-odr-violations")
			endif()
			
		else()
			
			# icc
			if(CMAKE_CXX_COMPILER_ID STREQUAL "Intel")
				# '... was declared but never referenced'
				# While normally a sensible warning, it also fires when a member isn't used for
				# *all* instantiations of a template class, making the warning too annoying to
				# be useful
				add_cxxflag("-wd177")
				# 'external function definition with no prior declaration'
				# This gets annoying fast with small inline/template functions.
				add_cxxflag("-wd1418")
				# 'non-pointer conversion from "int" to "…" may lose significant bits'
				add_cxxflag("-wd2259")
			endif()
			
			# -Wuninitialized causes too many false positives in older gcc versions
			if(CMAKE_COMPILER_IS_GNUCXX)
				# GCC is 'clever' and silently accepts -Wno-*  - check for the non-negated variant
				check_compiler_flag(FLAG_FOUND "-Wmaybe-uninitialized")
				if(FLAG_FOUND)
					add_cxxflag("-Wno-maybe-uninitialized")
				else()
					add_cxxflag("-Wno-uninitialized")
				endif()
			endif()
			
			# Xcode does not support -isystem yet
			if(MACOS)
				add_cxxflag("-Wno-undef")
			endif()
			
		endif()
		
		if(IS_LIBCXX)
			add_definitions(-D_LIBCPP_ENABLE_NODISCARD)
		endif()
		
	endif(SET_WARNING_FLAGS)
	
	if(WERROR)
		add_cxxflag("-Werror")
	endif()
	
	if(DEBUG_EXTRA)
		add_cxxflag("-ftrapv") # to add checks for (undefined) signed integer overflow
		add_cxxflag("-fbounds-checking")
		add_cxxflag("-fcatch-undefined-behavior")
		add_cxxflag("-fstack-protector-all")
		add_cxxflag("-fsanitize=address")
		# add_cxxflag("-fsanitize=thread") does not work together with -fsanitize=address
		add_cxxflag("-fsanitize=leak")
		add_cxxflag("-fsanitize=undefined")
		if(ARX_HAVE_LIBCPP_HARDENING_MODE)
			# libc++ 18+
			add_definitions(-D_LIBCPP_HARDENING_MODE=_LIBCPP_HARDENING_MODE_DEBUG)
		elseif(ARX_HAVE_LIBCPP_ENABLE_HARDENED_MODE)
			# libc++ 17 - Full debug mode is now a compile-time option and all -D_LIBCPP_DEBUG=1 does is
			# generate an #error if the library was not built in debug mode :|
			add_definitions(-D_LIBCPP_ENABLE_HARDENED_MODE=1)
		elseif(ARX_HAVE_LIBCPP_ENABLE_ASSERTIONS)
			# libc++ 15-16 - Full debug mode is now a compile-time option and all -D_LIBCPP_DEBUG=1 does is
			# generate an #error if the library was not built in debug mode :|
			add_definitions(-D_LIBCPP_ENABLE_ASSERTIONS=1)
		elseif(IS_LIBCXX)
			# older libc++
			add_definitions(-D_LIBCPP_DEBUG=1)
		else()
			# libstdc++
			add_definitions(-D_GLIBCXX_DEBUG -D_GLIBCXX_DEBUG_PEDANTIC -D_GLIBCXX_SANITIZE_VECTOR)
			set(disable_libstdcxx_debug "-U_GLIBCXX_DEBUG -U_GLIBCXX_DEBUG_PEDANTIC")
		endif()
	elseif(DEBUG)
		if(ARX_HAVE_LIBCPP_HARDENING_MODE)
			#libc++ 18+
			add_definitions(-D_LIBCPP_HARDENING_MODE=_LIBCPP_HARDENING_MODE_EXTENSIVE)
		elseif(ARX_HAVE_LIBCPP_ENABLE_HARDENED_MODE)
			# libc++ 17
			add_definitions(-D_LIBCPP_ENABLE_HARDENED_MODE=1)
		elseif(ARX_HAVE_LIBCPP_ENABLE_ASSERTIONS)
			# libc++ 15-16
			add_definitions(-D_LIBCPP_ENABLE_ASSERTIONS=1)
		elseif(IS_LIBCXX)
			# older libc++ - 0 means light checks only, it does not mean no checks
			add_definitions(-D_LIBCPP_DEBUG=0)
		else()
			# libstdc++
			add_definitions(-D_GLIBCXX_ASSERTIONS=1)
		endif()
	endif()
	
	if(CMAKE_BUILD_TYPE STREQUAL "")
		set(CMAKE_BUILD_TYPE "Release")
	endif()
	
	if(SET_OPTIMIZATION_FLAGS)
		
		if(MACOS)
			# TODO For some reason this check succeeds on macOS, but then
			# flag causes the actual build to fail :(
		else()
			# Link as few libraries as possible
			# This is much easier than trying to decide which libraries are needed for each
			# system
			add_ldflag("-Wl,--as-needed")
		endif()
		
		if(CMAKE_BUILD_TYPE STREQUAL "Debug")
			
			# set debug symbol level to -g3
			check_compiler_flag(RESULT "-g3")
			if(NOT RESULT STREQUAL "")
				string(REGEX REPLACE "(^| )-g(|[0-9]|gdb)" "\\1" CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
				set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} ${RESULT}")
			endif()
			
			# disable optimizations
			check_compiler_flag(RESULT "-Og")
			if(NOT RESULT)
				check_compiler_flag(RESULT "-O0")
			endif()
			string(REGEX REPLACE "-O[0-9]" "" CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
			set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} ${RESULT}")
			
		elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
			
			if((NOT CMAKE_CXX_FLAGS MATCHES "-g(|[0-9]|gdb)")
			   AND (NOT CMAKE_CXX_FLAGS_RELEASE MATCHES "-g(|[0-9]|gdb)"))
				add_cxxflag("-g2")
			endif()
			
			add_cxxflag("-ffast-math")
			
		endif()
		
	endif(SET_OPTIMIZATION_FLAGS)
	
endif(MSVC)
