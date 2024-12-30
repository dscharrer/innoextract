cmake_minimum_required(VERSION 2.8...3.19)

if(CMAKE_VERSION VERSION_LESS 3.12)
	cmake_policy(VERSION ${CMAKE_VERSION})
endif()

project(innoextract)


# Define configuration options

if(CMAKE_SYSTEM_NAME MATCHES "Darwin")
	set(MACOS 1)
else()
	set(MACOS 0)
endif()

macro(suboption _var _comment _type _default)
	if(NOT DEFINED ${_var})
		set(${_var} "${_default}")
	else()
		set(${_var} "${${_var}}" CACHE ${_type} "${_comment}")
	endif()
endmacro()

option(DEVELOPER "Use build settings suitable for developers" OFF)
option(CONTINUOUS_INTEGRATION "Use build settings suitable for CI" OFF)

# Components
set(default_BUILD_TESTS OFF)
if(CONTINUOUS_INTEGRATION OR DEVELOPER)
	set(default_BUILD_TESTS ON)
endif()
suboption(BUILD_TESTS "Build tests" BOOL ${default_BUILD_TESTS})
option(USE_ARC4 "Build ARC4 decryption support" ON)

# Optional dependencies
option(USE_LZMA "Build LZMA decompression support" ON)
option(USE_DYNAMIC_UTIMENSAT "Dynamically load utimensat if not available at compile time" OFF)

# Alternative dependencies
set(WITH_CONV CACHE STRING "The library to use for charset conversions")

# Build types
option(DEBUG_EXTRA "Expensive debug options" OFF)
option(SET_WARNING_FLAGS "Adjust compiler warning flags" ON)
option(SET_NOISY_WARNING_FLAGS "Enable noisy compiler warnings" OFF)
option(SET_OPTIMIZATION_FLAGS "Adjust compiler optimization flags" ON)
suboption(USE_LDGOLD "Use the Gold linker" BOOL ${SET_OPTIMIZATION_FLAGS})
set(default_FASTLINK OFF)
if(DEVELOPER OR CONTINUOUS_INTEGRATION)
	set(default_FASTLINK ON)
endif()
suboption(FASTLINK "Optimize (incremental) linking speed" BOOL ${default_FASTLINK})
set(default_USE_LTO OFF)
if(SET_OPTIMIZATION_FLAGS AND NOT FASTLINK)
	set(default_USE_LTO ON)
endif()
suboption(USE_LTO "Use link-time code generation" BOOL ${default_USE_LTO})
suboption(WERROR "Turn warnings into errors" BOOL ${CONTINUOUS_INTEGRATION})
suboption(CXX_STD_VERSION "Maximum C++ standard version to enable" STRING 2017)
if(DEVELOPER OR CMAKE_BUILD_TYPE STREQUAL "Debug")
	set(default_DEBUG ON)
else()
	set(default_DEBUG OFF)
endif()
suboption(DEBUG "Build with debug output" BOOL ${default_DEBUG})
if(DEBUG)
	add_definitions(-DDEBUG=1)
endif()

set(default_USE_STATIC_LIBS OFF)
if(WIN32)
	set(default_USE_STATIC_LIBS ON)
endif()
option(USE_STATIC_LIBS       "Statically link libraries" ${default_USE_STATIC_LIBS})
option(LZMA_USE_STATIC_LIBS  "Statically link liblzma"   ${USE_STATIC_LIBS})
option(ZLIB_USE_STATIC_LIBS  "Statically link libz"      ${USE_STATIC_LIBS})
option(BZip2_USE_STATIC_LIBS "Statically link libbz2"    ${USE_STATIC_LIBS})
option(Boost_USE_STATIC_LIBS "Statically link Boost"     ${USE_STATIC_LIBS})
option(iconv_USE_STATIC_LIBS "Statically link libiconv"  ${USE_STATIC_LIBS})

# Make optional dependencies required
suboption(STRICT_USE "Abort if there are missing optional dependencies" BOOL ${CONTINUOUS_INTEGRATION})
if(STRICT_USE)
	set(OPTIONAL_DEPENDENCY REQUIRED)
else()
	set(OPTIONAL_DEPENDENCY)
endif()

# Test configuration
set(RUN_TARGET CACHE STRING "Wrapper to run built targets")
mark_as_advanced(RUN_TARGET)
set(default_RUN_TESTS OFF)
if((DEVELOPER OR CONTINUOUS_INTEGRATION) AND (NOT CMAKE_CROSSCOMPILING OR NOT RUN_TARGET STREQUAL ""))
	set(default_RUN_TESTS ON)
endif()
suboption(RUN_TESTS "Run tests as part of the default build target" BOOL ${default_RUN_TESTS})

# Install destinations
if(CMAKE_VERSION VERSION_LESS 2.8.5)
	set(CMAKE_INSTALL_DATAROOTDIR "share" CACHE
	    STRING "read-only architecture-independent data root (share) (relative to prefix).")
	set(CMAKE_INSTALL_BINDIR "bin" CACHE
	    STRING "user executables (bin) (relative to prefix).")
	set(CMAKE_INSTALL_MANDIR "${CMAKE_INSTALL_DATAROOTDIR}/man" CACHE
	    STRING "man documentation (DATAROOTDIR/man) (relative to prefix).")
	mark_as_advanced(
		CMAKE_INSTALL_DATAROOTDIR
		CMAKE_INSTALL_BINDIR
		CMAKE_INSTALL_MANDIR
	)
else()
	include(GNUInstallDirs)
endif()


# Helper scrips

include(CheckSymbolExists)

set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake") # For custom cmake modules
include(BuildType)
include(CompileCheck)
include(CreateSourceGroups)
include(CXXVersionCheck)
include(Doxygen)
include(FilterList)
include(PrintConfiguration)
include(StyleCheck)
include(UseStaticLibs)
include(VersionString)


# Find required libraries

# Win32 API
if(WIN32)
	# Ensure we aren't using functionalities not found under Window XP SP1
	add_definitions(-D_WIN32_WINNT=0x0502)
	add_definitions(-DNOMINMAX)
	add_definitions(-DWIN32_LEAN_AND_MEAN)
endif()

if(USE_STATIC_LIBS AND NOT MSVC)
	add_ldflag("-static-libstdc++")
	add_ldflag("-static-libgcc")
endif()

unset(LIBRARIES)

if(USE_ARC4)
	set(INNOEXTRACT_HAVE_ARC4 1)
endif()

if(USE_LZMA)
	find_package(LZMA REQUIRED)
	list(APPEND LIBRARIES ${LZMA_LIBRARIES})
	include_directories(SYSTEM ${LZMA_INCLUDE_DIR})
	add_definitions(${LZMA_DEFINITIONS})
	set(INNOEXTRACT_HAVE_LZMA 1)
else()
	message(WARNING "\nDisabling LZMA decompression support.\n"
	                "You won't be able to extract most newer Inno Setup installers.")
	set(INNOEXTRACT_HAVE_LZMA 0)
endif()

find_package(Boost REQUIRED COMPONENTS
	iostreams
	filesystem
	date_time
	system
	program_options
)
list(APPEND LIBRARIES ${Boost_LIBRARIES})
link_directories(${Boost_LIBRARY_DIRS})
include_directories(SYSTEM ${Boost_INCLUDE_DIR})
if(NOT Boost_VERSION_MACRO)
	# CMP0093 changed Boost_VERSION to x.y.z format and provide the old format in Boost_VERSION_MACRO
	set(Boost_VERSION_MACRO ${Boost_VERSION})
endif()

has_static_libs(Boost Boost_LIBRARIES)
if(Boost_HAS_STATIC_LIBS)
	foreach(Lib IN ITEMS ZLIB BZip2)
		string(TOUPPER ${Lib} LIB)
		string(TOLOWER ${Lib} lib)
		foreach(static IN ITEMS 1 0)
			if(static)
				use_static_libs(${Lib})
			endif()
			if(WIN32)
				find_package(Boost COMPONENTS ${lib} QUIET)
			endif()
			if(Boost_${LIB}_FOUND)
				message (STATUS "Found boost_${lib}")
				set(${LIB}_LIBRARIES ${Boost_${LIB}_LIBRARY})
			else()
				find_package(${Lib} REQUIRED)
			endif()
			if(static)
				use_static_libs_restore()
			endif()
			if(${LIB}_LIBRARIES OR STRICT_USE)
				break()
			endif()
		endforeach()
		list(APPEND LIBRARIES ${${LIB}_LIBRARIES})
	endforeach()
endif()

set(INNOEXTRACT_HAVE_ICONV 0)
set(INNOEXTRACT_HAVE_WIN32_CONV 0)
if(WIN32 AND (NOT WITH_CONV OR WITH_CONV STREQUAL "win32"))
	set(INNOEXTRACT_HAVE_WIN32_CONV 1)
elseif(NOT WITH_CONV OR WITH_CONV STREQUAL "iconv")
	if(STRICT_USE)
		set(ICONV_REQUIRED REQUIRED)
	else()
		set(ICONV_REQUIRED)
	endif()
	find_package(iconv ${ICONV_REQUIRED})
	if(ICONV_FOUND)
		list(APPEND LIBRARIES ${iconv_LIBRARIES})
		include_directories(SYSTEM ${iconv_INCLUDE_DIR})
		add_definitions(${iconv_DEFINITIONS})
		set(INNOEXTRACT_HAVE_ICONV 1)
	endif()
elseif(WITH_CONV AND NOT WITH_CONV STREQUAL "builtin")
	message(FATAL_ERROR "Invalid WITH_CONV option: ${WITH_CONV}")
endif()


# Set compiler flags

if(Boost_VERSION_MACRO LESS 104800)
	# Older Boost versions don't work with C++11
elseif(NOT CXX_STD_VERSION LESS 2011)
	enable_cxx_version(${CXX_STD_VERSION})
	check_cxx11("alignof" INNOEXTRACT_HAVE_ALIGNOF)
	if(WIN32)
		check_cxx11("std::codecvt_utf8_utf16" INNOEXTRACT_HAVE_STD_CODECVT_UTF8_UTF16 1600)
	endif()
	check_cxx11("std::unique_ptr" INNOEXTRACT_HAVE_STD_UNIQUE_PTR 1600)
endif()

# Don't expose internal symbols to the outside world by default
if(NOT MSVC)
	add_cxxflag("-fvisibility=hidden")
	add_cxxflag("-fvisibility-inlines-hidden")
endif()

# Older glibc versions won't provide some useful symbols by default - request them
# This flag is currently also set by gcc when compiling C++, but not for plain C
if(NOT WIN32)
	check_symbol_exists(__GLIBC__ "features.h" HAVE_GLIBC)
	if(HAVE_GLIBC)
		set(CMAKE_REQUIRED_DEFINITIONS "-D_GNU_SOURCE=1")
		add_definitions(-D_GNU_SOURCE=1)
	endif()
endif()

if(WIN32)
	# Define this so that we don't accitenally use ANSI functions
	add_definitions(-DUNICODE)
	add_definitions(-D_UNICODE)
endif()


# Check for optional functionality and system configuration

if(NOT WIN32)
	
	check_symbol_exists(isatty "unistd.h" INNOEXTRACT_HAVE_ISATTY)
	check_symbol_exists(ioctl "sys/ioctl.h" INNOEXTRACT_HAVE_IOCTL)
	check_symbol_exists(timegm "time.h" INNOEXTRACT_HAVE_TIMEGM)
	check_symbol_exists(gmtime_r "time.h" INNOEXTRACT_HAVE_GMTIME_R)
	check_symbol_exists(AT_FDCWD "fcntl.h" INNOEXTRACT_HAVE_AT_FDCWD)
	if(INNOEXTRACT_HAVE_AT_FDCWD)
		check_symbol_exists(utimensat "sys/stat.h" INNOEXTRACT_HAVE_UTIMENSAT)
	endif()
	if(INNOEXTRACT_HAVE_UTIMENSAT AND INNOEXTRACT_HAVE_AT_FDCWD)
		set(INNOEXTRACT_HAVE_UTIMENSAT_d 1)
	else()
		if(USE_DYNAMIC_UTIMENSAT AND INNOEXTRACT_HAVE_AT_FDCWD)
			set(CMAKE_REQUIRED_LIBRARIES ${CMAKE_DL_LIBS})
			check_symbol_exists(dlsym "dlfcn.h" INNOEXTRACT_HAVE_DLSYM)
			check_symbol_exists(RTLD_DEFAULT "dlfcn.h" INNOEXTRACT_HAVE_RTLD_DEFAULT)
			unset(CMAKE_REQUIRED_LIBRARIES)
			if(INNOEXTRACT_HAVE_DLSYM AND INNOEXTRACT_HAVE_RTLD_DEFAULT)
				set(INNOEXTRACT_HAVE_DYNAMIC_UTIMENSAT 1)
				if(CMAKE_DL_LIBS)
					list(APPEND LIBRARIES ${CMAKE_DL_LIBS})
				endif()
			endif()
		endif()
		check_symbol_exists(utimes "sys/time.h" INNOEXTRACT_HAVE_UTIMES)
	endif()
	check_symbol_exists(posix_spawnp "spawn.h" INNOEXTRACT_HAVE_POSIX_SPAWNP)
	if(INNOEXTRACT_HAVE_POSIX_SPAWNP)
		check_symbol_exists(environ "unistd.h" INNOEXTRACT_HAVE_UNISTD_ENVIRON)
	else()
		check_symbol_exists(fork "unistd.h" INNOEXTRACT_HAVE_FORK)
		check_symbol_exists(execvp "unistd.h" INNOEXTRACT_HAVE_EXECVP)
	endif()
	if(INNOEXTRACT_HAVE_POSIX_SPAWNP OR (INNOEXTRACT_HAVE_FORK AND INNOEXTRACT_HAVE_EXECVP))
		check_symbol_exists(waitpid "sys/wait.h" INNOEXTRACT_HAVE_WAITPID)
	endif()
	
endif()

if(NOT MSVC)
	
	if(CMAKE_CXX_COMPILER_ID STREQUAL "PathScale")
		# EKOPath recognizes these but then fails to link
	else()
		check_builtin(INNOEXTRACT_HAVE_BUILTIN_BSWAP16 "__builtin_bswap16(0)")
		check_builtin(INNOEXTRACT_HAVE_BUILTIN_BSWAP32 "__builtin_bswap32(0)")
		check_builtin(INNOEXTRACT_HAVE_BUILTIN_BSWAP64 "__builtin_bswap64(0)")
	endif()
	if(NOT INNOEXTRACT_HAVE_BUILTIN_BSWAP16)
		check_symbol_exists(bswap_16 "byteswap.h" INNOEXTRACT_HAVE_BSWAP_16)
	endif()
	if(NOT INNOEXTRACT_HAVE_BUILTIN_BSWAP32)
		check_symbol_exists(bswap_32 "byteswap.h" INNOEXTRACT_HAVE_BSWAP_32)
	endif()
	if(NOT INNOEXTRACT_HAVE_BUILTIN_BSWAP64)
		check_symbol_exists(bswap_64 "byteswap.h" INNOEXTRACT_HAVE_BSWAP_64)
	endif()
	
endif()

if($ENV{PORTAGE_REPO_NAME} MATCHES "gentoo")
	# Meh
	unset(LIBRARIES)
endif()


# All sources:

set(DOCUMENTATION 0) # never build these

set(INNOEXTRACT_SOURCES
	
	src/index.hpp if DOCUMENTATION
	src/release.hpp
	
	src/cli/debug.hpp
	src/cli/debug.cpp if DEBUG
	src/cli/extract.hpp
	src/cli/extract.cpp
	src/cli/gog.hpp
	src/cli/gog.cpp
	src/cli/goggalaxy.hpp
	src/cli/goggalaxy.cpp
	src/cli/main.cpp
	
	src/crypto/adler32.hpp
	src/crypto/adler32.cpp
	src/crypto/arc4.hpp if INNOEXTRACT_HAVE_ARC4
	src/crypto/arc4.cpp if INNOEXTRACT_HAVE_ARC4
	src/crypto/checksum.hpp
	src/crypto/checksum.cpp
	src/crypto/crc32.hpp
	src/crypto/crc32.cpp
	src/crypto/hasher.cpp
	src/crypto/hasher.cpp
	src/crypto/iteratedhash.hpp
	src/crypto/md5.hpp
	src/crypto/md5.cpp
	src/crypto/sha1.hpp
	src/crypto/sha1.cpp
	
	src/loader/exereader.hpp
	src/loader/exereader.cpp
	src/loader/offsets.hpp
	src/loader/offsets.cpp
	
	src/setup/component.hpp
	src/setup/component.cpp
	src/setup/data.hpp
	src/setup/data.cpp
	src/setup/delete.hpp
	src/setup/delete.cpp
	src/setup/directory.hpp
	src/setup/directory.cpp
	src/setup/expression.hpp
	src/setup/expression.cpp
	src/setup/file.hpp
	src/setup/file.cpp
	src/setup/filename.hpp
	src/setup/filename.cpp
	src/setup/header.hpp
	src/setup/header.cpp
	src/setup/icon.hpp
	src/setup/icon.cpp
	src/setup/info.hpp
	src/setup/info.cpp
	src/setup/ini.hpp
	src/setup/ini.cpp
	src/setup/item.hpp
	src/setup/item.cpp
	src/setup/language.hpp
	src/setup/language.cpp
	src/setup/message.hpp
	src/setup/message.cpp
	src/setup/permission.hpp
	src/setup/permission.cpp
	src/setup/registry.hpp
	src/setup/registry.cpp
	src/setup/run.hpp
	src/setup/run.cpp
	src/setup/task.hpp
	src/setup/task.cpp
	src/setup/type.hpp
	src/setup/type.cpp
	src/setup/version.hpp
	src/setup/version.cpp
	src/setup/windows.hpp
	src/setup/windows.cpp
	
	src/stream/block.hpp
	src/stream/block.cpp
	src/stream/checksum.hpp
	src/stream/chunk.hpp
	src/stream/chunk.cpp
	src/stream/exefilter.hpp
	src/stream/file.hpp
	src/stream/file.cpp
	src/stream/lzma.hpp
	src/stream/lzma.cpp if INNOEXTRACT_HAVE_LZMA
	src/stream/restrict.hpp
	src/stream/slice.hpp
	src/stream/slice.cpp
	
	src/util/align.hpp
	src/util/ansi.hpp
	src/util/boostfs_compat.hpp
	src/util/console.hpp
	src/util/console.cpp
	src/util/encoding.hpp
	src/util/encoding.cpp
	src/util/endian.hpp
	src/util/enum.hpp
	src/util/flags.hpp
	src/util/fstream.hpp
	src/util/load.hpp
	src/util/load.cpp
	src/util/log.hpp
	src/util/log.cpp
	src/util/math.hpp
	src/util/output.hpp
	src/util/process.hpp
	src/util/process.cpp
	src/util/storedenum.hpp
	src/util/time.hpp
	src/util/time.cpp
	src/util/test.hpp
	src/util/types.hpp
	src/util/unique_ptr.hpp
	src/util/windows.hpp
	src/util/windows.cpp if WIN32
	
)

set(UNITTEST_SOURCES
	
	src/util/test.hpp
	src/util/test.cpp
	
)

filter_list(INNOEXTRACT_SOURCES ALL_INNOEXTRACT_SOURCES)
filter_list(UNITTEST_SOURCES ALL_UNITTEST_SOURCES)

create_source_groups(ALL_INNOEXTRACT_SOURCES)


# Prepare generated files

include_directories(src ${CMAKE_CURRENT_BINARY_DIR})

configure_file("src/configure.hpp.in" "configure.hpp")

set(VERSION_FILE "${PROJECT_BINARY_DIR}/release.cpp")
set(VERSION_SOURCES VERSION "VERSION" LICENSE "LICENSE")
version_file("src/release.cpp.in" ${VERSION_FILE} "${VERSION_SOURCES}" ".git")
list(APPEND INNOEXTRACT_SOURCES ${VERSION_FILE})

set(MAN_INPUT "doc/innoextract.1.in")
set(MAN_FILE "${PROJECT_BINARY_DIR}/innoextract.1")
set(MAN_SOURCES VERSION "VERSION" CHANGELOG "CHANGELOG")
version_file(${MAN_INPUT} ${MAN_FILE} "${MAN_SOURCES}" ".git")
add_custom_target(manpage ALL DEPENDS ${MAN_FILE})


# Main targets

add_executable(innoextract ${INNOEXTRACT_SOURCES})
target_link_libraries(innoextract ${LIBRARIES})

install(TARGETS innoextract RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

install(FILES ${MAN_FILE} DESTINATION ${CMAKE_INSTALL_MANDIR}/man1 OPTIONAL)


# Test target

if(BUILD_TESTS)
	
	enable_testing()
	
	set(run_tests)
	if(RUN_TESTS)
		set(run_tests ALL)
	endif()
	
	add_executable(unittest ${UNITTEST_SOURCES})
	target_link_libraries(unittest ${LIBRARIES})
	target_compile_definitions(unittest PRIVATE INNOEXTRACT_BUILD_TESTS)
	
	set(unittest_binary "$<TARGET_FILE:unittest>")
	
	add_test(NAME "unit tests"
		COMMAND ${RUN_TARGET} "${unittest_binary}"
		WORKING_DIRECTORY "${PROJECT_BINARY_DIR}"
	)
	
	add_custom_command(
		OUTPUT "${PROJECT_BINARY_DIR}/unittest.check"
		COMMAND ${RUN_TARGET} "${unittest_binary}"
		COMMAND ${CMAKE_COMMAND} -E touch "${PROJECT_BINARY_DIR}/unittest.check"
		DEPENDS unittest
		WORKING_DIRECTORY "${PROJECT_BINARY_DIR}"
		COMMENT "Running unit tests" VERBATIM
	)
	
	add_custom_target(check ${run_tests}
		DEPENDS "${PROJECT_BINARY_DIR}/unittest.check"
	)
	
endif()


# Additional targets.

add_style_check_target(style "${ALL_INNOEXTRACT_SOURCES}" innoextract)

add_doxygen_target(doc "doc/Doxyfile.in" "VERSION" ".git" "${PROJECT_BINARY_DIR}/doc")


# Print a configuration summary

message("")
message("Configuration:")
set(BUILD_TYPE_SUFFIX "")
if(DEBUG AND NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
	set(BUILD_TYPE_SUFFIX "${BUILD_TYPE_SUFFIX} with debug output")
elseif(NOT DEBUG AND NOT CMAKE_BUILD_TYPE STREQUAL "Release")
	set(BUILD_TYPE_SUFFIX "${BUILD_TYPE_SUFFIX} without debug output")
endif()
message(" - Build type: ${CMAKE_BUILD_TYPE}${BUILD_TYPE_SUFFIX}")
print_configuration("ARC4 decryption" FIRST
	INNOEXTRACT_HAVE_ARC4 "enabled"
	1                     "disabled"
)
print_configuration("LZMA decompression" FIRST
	INNOEXTRACT_HAVE_LZMA "enabled"
	1                     "disabled"
)
if(INNOEXTRACT_HAVE_DYNAMIC_UTIMENSAT)
	set(time_prefix "nanoseconds if supported, ")
	set(time_suffix " otherwise")
endif()
print_configuration("File time precision" FIRST
	INNOEXTRACT_HAVE_UTIMENSAT_d "nanoseconds"
	WIN32                        "100-nanoseconds"
	INNOEXTRACT_HAVE_UTIMES      "${time_prefix}microseconds${time_suffix}"
	1                            "${time_prefix}seconds${time_suffix}"
)
print_configuration("Charset conversion"
	INNOEXTRACT_HAVE_ICONV        "iconv"
	INNOEXTRACT_HAVE_WIN32_CONV   "Win32"
	1                             "builtin"
)
message("")

if(DEVELOPER)
	
	file(READ "README.md" readme)
	parse_version_file("VERSION" "VERSION")
	string(REPLACE "${VERSION_2}" "" readme_without_version "${readme}")
	if(readme_without_version STREQUAL readme)
		message(WARNING "Could not find '${VERSION_2}' in README.md.")
	endif()
	
	foreach(file IN LISTS ALL_INNOEXTRACT_SOURCES)
		file(READ "${file}" source)
		if(source MATCHES ".*INNOEXTRACT_TEST.*")
			list(FIND ALL_UNITTEST_SOURCES ${file} result)
			if(result EQUAL -1)
				message(WARNING "Could not find '${file}' in UNITTEST_SOURCES")
			endif()
		endif()
	endforeach()
	
endif()
