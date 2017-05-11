project(innoextract)

cmake_minimum_required(VERSION 2.8)

if(CMAKE_VERSION VERSION_GREATER 3.4)
	cmake_policy(VERSION 3.4)
else()
	cmake_policy(VERSION ${CMAKE_VERSION})
endif()


# Define configuration options

option(STRICT_USE "Abort if there are missing optional dependencies"  OFF)
option(USE_LZMA "Build lzma decompression support" ON)
set(WITH_CONV CACHE STRING "The library to use for charset conversions")
option(ENABLE_BUILTIN_CONV "Build internal charset conversion routines" ON)
option(DEBUG_EXTRA "Expensive debug options" OFF)
option(SET_WARNING_FLAGS "Adjust compiler warning flags" ON)
option(SET_OPTIMIZATION_FLAGS "Adjust compiler optimization flags" ON)
option(USE_CXX11 "Try to use C++11 if available" ON)
option(USE_DYNAMIC_UTIMENSAT "Dynamically load utimensat if not available at compile time" OFF)
if(NOT DEFINED DEBUG AND CMAKE_BUILD_TYPE STREQUAL "Debug")
	set(DEBUG 1)
endif()
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
include(CXX11Check)
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

# Force re-checking libraries if the compiler or compiler flags change
if((NOT LAST_CMAKE_CXX_FLAGS STREQUAL CMAKE_CXX_FLAGS)
   OR (NOT LAST_CMAKE_CXX_COMPILER STREQUAL CMAKE_CXX_COMPILER))
	force_recheck_library(LZMA)
	force_recheck_library(Boost)
	force_recheck_library(ZLIB)
	force_recheck_library(BZip2)
	force_recheck_library(iconv)
	unset(Boost_INCLUDE_DIR CACHE)
	set(LAST_CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}" CACHE INTERNAL
	    "The last C++ compiler flags")
	set(LAST_CMAKE_CXX_COMPILER "${CMAKE_CXX_COMPILER}" CACHE INTERNAL
	    "The last C++ compiler")
endif()

unset(LIBRARIES)

list(APPEND LIBRARIES "iconv")

if(USE_LZMA)
	find_package(LZMA REQUIRED)
	check_link_library(LZMA LZMA_LIBRARIES)
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
check_link_library(Boost Boost_LIBRARIES)
list(APPEND LIBRARIES ${Boost_LIBRARIES})
link_directories(${Boost_LIBRARY_DIRS})
include_directories(SYSTEM ${Boost_INCLUDE_DIR})

has_static_libs(Boost Boost_LIBRARIES)
if(Boost_HAS_STATIC_LIBS)
	
	use_static_libs(ZLIB)
	find_package(ZLIB REQUIRED)
	use_static_libs_restore()
	check_link_library(ZLIB ZLIB_LIBRARIES)
	list(APPEND LIBRARIES ${ZLIB_LIBRARIES})
	
	use_static_libs(BZip2)
	find_package(BZip2 REQUIRED)
	use_static_libs_restore()
	check_link_library(BZip2 BZIP2_LIBRARIES)
	list(APPEND LIBRARIES ${BZIP2_LIBRARIES})
	
endif()

set(INNOEXTRACT_HAVE_ICONV 0)
set(INNOEXTRACT_HAVE_WIN32_CONV 0)
set(INNOEXTRACT_HAVE_BUILTIN_CONV ${ENABLE_BUILTIN_CONV})
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
		check_link_library(iconv iconv_LIBRARIES)
		list(APPEND LIBRARIES ${iconv_LIBRARIES})
		include_directories(SYSTEM ${iconv_INCLUDE_DIR})
		add_definitions(${iconv_DEFINITIONS})
		set(INNOEXTRACT_HAVE_ICONV 1)
	endif()
elseif(WITH_CONV AND NOT WITH_CONV STREQUAL "builtin")
	message(FATAL_ERROR "Invalid WITH_CONV option: ${WITH_CONV}")
endif()
if(NOT INNOEXTRACT_HAVE_ICONV AND NOT INNOEXTRACT_HAVE_WIN32_CONV
   AND NOT INNOEXTRACT_HAVE_BUILTIN_CONV)
	message(WARNING "\nBuilding without any charset conversion support.\n"
	                "Any non-ASCII characters in extracted filenames will be missing.")
endif()


# Set compiler flags

if(Boost_VERSION LESS 104800)
	# Older Boost versions don't work with C++11
elseif(USE_CXX11)
	enable_cxx11()
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
if(CMAKE_SYSTEM_NAME MATCHES "Linux")
	set(CMAKE_REQUIRED_DEFINITIONS "-D_GNU_SOURCE=1")
	add_definitions(-D_GNU_SOURCE=1)
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
	if(NOT INNOEXTRACT_HAVE_POSIX_SPAWNP)
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
	src/cli/main.cpp
	
	src/crypto/adler32.hpp
	src/crypto/adler32.cpp
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
	src/util/types.hpp
	src/util/unique_ptr.hpp
	src/util/windows.hpp
	src/util/windows.cpp if WIN32
	
)

filter_list(INNOEXTRACT_SOURCES ALL_INNOEXTRACT_SOURCES)

create_source_groups(ALL_INNOEXTRACT_SOURCES)


# Prepare generated files

include_directories(src ${CMAKE_CURRENT_BINARY_DIR})

configure_file("src/configure.hpp.in" "configure.hpp")

set(VERSION_FILE "${PROJECT_BINARY_DIR}/release.cpp")
set(VERSION_SOURCES VERSION "VERSION" LICENSE "LICENSE")
version_file("src/release.cpp.in" "${VERSION_FILE}" "${VERSION_SOURCES}" ".git")
list(APPEND INNOEXTRACT_SOURCES ${VERSION_FILE})


# Main targets

add_executable(innoextract ${INNOEXTRACT_SOURCES})
target_link_libraries(innoextract ${LIBRARIES})

install(TARGETS innoextract RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

install(FILES doc/innoextract.1 DESTINATION ${CMAKE_INSTALL_MANDIR}/man1 OPTIONAL)


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
	INNOEXTRACT_HAVE_BUILTIN_CONV "builtin"
)
message("")
