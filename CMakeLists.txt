project(innoextract)

cmake_minimum_required(VERSION 2.8)


# Define configuration options

option(USE_LZMA "Build lzma decompression support." ON)
option(DEBUG_EXTRA "Expensive debug options" OFF)
option(SET_WARNING_FLAGS "Adjust compiler warning flags" ON)
option(SET_OPTIMIZATION_FLAGS "Adjust compiler optimization flags" ON)
option(USE_CXX11 "Try to use C++11 if available" ON)

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

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake") # For custom cmake modules
include(BuildType)
include(CompileCheck)
include(CXX11Check)
include(Doxygen)
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
	if(NOT MSVC)
		 # required for _gmtime64_s on MinGW
		add_definitions(-DMINGW_HAS_SECURE_API)
		list(APPEND CMAKE_REQUIRED_DEFINITIONS -DMINGW_HAS_SECURE_API)
	endif()
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

if(Boost_USE_STATIC_LIBS)
	
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

find_package(iconv REQUIRED)
check_link_library(iconv iconv_LIBRARIES)
list(APPEND LIBRARIES ${iconv_LIBRARIES})
include_directories(SYSTEM ${iconv_INCLUDE_DIR})
add_definitions(${iconv_DEFINITIONS})


# Set compiler flags

if(${Boost_VERSION} LESS 104800)
	# Older Boost versions don't work with C++11
elseif(USE_CXX11)
	enable_cxx11()
	check_cxx11("alignof" INNOEXTRACT_HAVE_ALIGNOF)
	if(WIN32)
		check_cxx11("std::codecvt_utf8_utf16" INNOEXTRACT_HAVE_STD_CODECVT_UTF8_UTF16)
	endif()
	check_cxx11("std::unique_ptr" INNOEXTRACT_HAVE_STD_UNIQUE_PTR)
endif()

# Don't expose internal symbols to the outside world by default
if(NOT MSVC)
	add_cxxflag("-fvisibility=hidden")
	add_cxxflag("-fvisibility-inlines-hidden")
endif()

# Older glibc versions won't provide some useful symbols by default - request them
# This flag is currently also set by gcc when compiling C++, but not for plain C
if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	set(CMAKE_REQUIRED_DEFINITIONS "-D_GNU_SOURCE=1")
	add_definitions(-D_GNU_SOURCE=1)
endif()

if(WIN32)
	# Define this so that we don't accitenally use ANSI functions
	add_definitions(-D_UNICODE)
endif()


# Check for optional functionality and system configuration

check_symbol_exists(isatty "unistd.h" INNOEXTRACT_HAVE_ISATTY)
if(NOT INNOEXTRACT_HAVE_ISATTY)
	check_symbol_exists(_isatty "io.h" INNOEXTRACT_HAVE_MS_ISATTY)
endif()
check_symbol_exists(ioctl "sys/ioctl.h" INNOEXTRACT_HAVE_IOCTL)
check_symbol_exists(_mkgmtime64 "time.h" INNOEXTRACT_HAVE_MKGMTIME64)
if(NOT INNOEXTRACT_HAVE_MKGMTIME64)
	check_symbol_exists(timegm "time.h" INNOEXTRACT_HAVE_TIMEGM)
	if(NOT INNOEXTRACT_HAVE_TIMEGM)
		check_symbol_exists(_mkgmtime "time.h" INNOEXTRACT_HAVE_MKGMTIME)
	endif()
endif()
check_symbol_exists(_gmtime64_s "time.h" INNOEXTRACT_HAVE_GMTIME64_S)
if(NOT INNOEXTRACT_HAVE_GMTIME64_S)
	check_symbol_exists(gmtime_r "time.h" INNOEXTRACT_HAVE_GMTIME_R)
	if(NOT INNOEXTRACT_HAVE_GMTIME_R)
		check_symbol_exists(gmtime_s "time.h" INNOEXTRACT_HAVE_GMTIME_S)
	endif()
endif()
if(NOT WIN32)
	check_symbol_exists(utimensat "sys/stat.h" INNOEXTRACT_HAVE_UTIMENSAT)
	check_symbol_exists(AT_FDCWD "fcntl.h" INNOEXTRACT_HAVE_AT_FDCWD)
	if(INNOEXTRACT_HAVE_UTIMENSAT AND INNOEXTRACT_HAVE_AT_FDCWD)
		set(INNOEXTRACT_HAVE_UTIMENSAT_d 1)
	else()
		check_symbol_exists(utimes "sys/time.h" INNOEXTRACT_HAVE_UTIMES)
	endif()
endif()

check_symbol_exists(bswap_16 "byteswap.h" INNOEXTRACT_HAVE_BSWAP_16)
check_symbol_exists(bswap_32 "byteswap.h" INNOEXTRACT_HAVE_BSWAP_32)
check_symbol_exists(bswap_64 "byteswap.h" INNOEXTRACT_HAVE_BSWAP_64)


# All sources:

set(INNOEXTRACT_SOURCES
	
	src/cli/main.cpp
	
	src/crypto/adler32.cpp
	src/crypto/checksum.cpp
	src/crypto/crc32.cpp
	src/crypto/hasher.cpp
	src/crypto/md5.cpp
	src/crypto/sha1.cpp
	
	src/loader/exereader.cpp
	src/loader/offsets.cpp
	
	src/setup/component.cpp
	src/setup/data.cpp
	src/setup/delete.cpp
	src/setup/directory.cpp
	src/setup/expression.cpp
	src/setup/file.cpp
	src/setup/filename.cpp
	src/setup/header.cpp
	src/setup/icon.cpp
	src/setup/info.cpp
	src/setup/ini.cpp
	src/setup/item.cpp
	src/setup/language.cpp
	src/setup/message.cpp
	src/setup/permission.cpp
	src/setup/registry.cpp
	src/setup/run.cpp
	src/setup/task.cpp
	src/setup/type.cpp
	src/setup/version.cpp
	src/setup/windows.cpp
	
	src/stream/block.cpp
	src/stream/chunk.cpp
	src/stream/file.cpp
	
	src/stream/slice.cpp
	src/util/console.cpp
	src/util/load.cpp
	src/util/log.cpp
	src/util/time.cpp
	
)

if(DEBUG)
	list(APPEND INNOEXTRACT_SOURCES src/cli/debug.cpp)
endif()

if(LZMA_FOUND)
	list(APPEND INNOEXTRACT_SOURCES src/stream/lzma.cpp)
endif()

if(WIN32)
	list(APPEND INNOEXTRACT_SOURCES src/util/windows.cpp)
endif()

file(GLOB_RECURSE ALL_INCLUDES "${CMAKE_SOURCE_DIR}/src/*.hpp")

list(SORT INNOEXTRACT_SOURCES)
list(SORT ALL_INCLUDES)

list(APPEND CHECKED_SOURCES ${INNOEXTRACT_SOURCES})


# Prepare generated files

include_directories(src ${CMAKE_CURRENT_BINARY_DIR})

configure_file("src/configure.hpp.in" "configure.hpp")

set(VERSION_FILE "${CMAKE_BINARY_DIR}/release.cpp")
set(VERSION_SOURCES VERSION "VERSION" LICENSE "LICENSE")
version_file("src/release.cpp.in" "${VERSION_FILE}" "${VERSION_SOURCES}" ".git")
list(APPEND INNOEXTRACT_SOURCES "${VERSION_FILE}")


# Main targets

add_executable(innoextract ${INNOEXTRACT_SOURCES} ${ALL_INCLUDES})
target_link_libraries(innoextract ${LIBRARIES})

install(TARGETS innoextract RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

install(FILES doc/innoextract.1 DESTINATION ${CMAKE_INSTALL_MANDIR}/man1 OPTIONAL)


# Additional targets.

add_style_check_target(style "${CHECKED_SOURCES}" "${ALL_INCLUDES}")

add_doxygen_target(doc "doc/Doxyfile.in" "VERSION" ".git" "${CMAKE_BINARY_DIR}/doc")


# Print a configuration summary

message("")
message("Configuration:")
message(" - Build type: ${CMAKE_BUILD_TYPE}")
print_configuration("LZMA decompression" FIRST
	INNOEXTRACT_HAVE_LZMA "enabled"
	1                     "disabled"
)
print_configuration("File time precision" FIRST
	INNOEXTRACT_HAVE_UTIMENSAT_d "nanoseconds"
	WIN32                        "100-nanoseconds"
	INNOEXTRACT_HAVE_UTIMES      "microseconds"
	1                            "seconds"
)
message("")
