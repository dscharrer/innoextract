project(InnoExtract)

cmake_minimum_required(VERSION 2.8)


# Define configuration options

option(USE_LZMA "Build lzma decompression support." ON)
option(DEBUG_EXTRA "Expensive debug options" OFF)
option(SET_WARNING_FLAGS "Adjust compiler warning flags" ON)
option(SET_OPTIMIZATION_FLAGS "Adjust compiler optimization flags" ON)
option(USE_CXX11 "Try to use C++11 if available" ON)

set(MAN_DIR "share/man" CACHE STRING "Install path for man pages (relative to prefix).")
mark_as_advanced(MAN_DIR)


# Helper scrips

include(CheckSymbolExists)
include(TestBigEndian)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake") # For custom cmake modules
include(BuildType)
include(CompileCheck)
include(Doxygen)
include(StyleCheck)
include(VersionString)


# Find required libraries

# Force re-checking libraries if the compiler or compiler flags change
if((NOT LAST_CMAKE_CXX_FLAGS STREQUAL CMAKE_CXX_FLAGS)
   OR (NOT LAST_CMAKE_CXX_COMPILER STREQUAL CMAKE_CXX_COMPILER))
	force_recheck_library(LZMA)
	force_recheck_library(Boost)
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
	list(APPEND LIBRARIES "${LZMA_LIBRARIES}")
	include_directories(SYSTEM "${LZMA_INCLUDE_DIR}")
	set(HAVE_LZMA 1)
else()
	message(WARNING "\nDisabling LZMA decompression support.\n"
	                "You won't be able to extract most newer Inno Setup installers.")
	set(HAVE_LZMA 0)
endif()

find_package(Boost REQUIRED COMPONENTS
	iostreams
	filesystem
	date_time
	system
	program_options
)
check_link_library(Boost Boost_LIBRARIES)
list(APPEND LIBRARIES "${Boost_LIBRARIES}")
link_directories("${Boost_LIBRARY_DIRS}")
include_directories(SYSTEM "${Boost_INCLUDE_DIR}")

find_package(iconv REQUIRED)
list(APPEND LIBRARIES "${iconv_LIBRARIES}")
include_directories(SYSTEM "${iconv_INCLUDE_DIR}")
check_link_library(iconv iconv_LIBRARIES)


# Set compiler flags

if(${Boost_VERSION} LESS 104800)
	# Older Boost versions don't work with C++11
elseif(USE_CXX11)
	add_cxxflag("-std=c++11")
endif()


# Check for optional functionality and system configuration

check_symbol_exists(isatty "unistd.h" HAVE_ISATTY)
check_symbol_exists(ioctl "sys/ioctl.h" HAVE_IOCTL)

test_big_endian(IS_BIG_ENDIAN)


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
	
)

if(DEBUG)
	list(APPEND INNOEXTRACT_SOURCES src/cli/debug.cpp)
endif()

if(LZMA_FOUND)
	list(APPEND INNOEXTRACT_SOURCES src/stream/lzma.cpp)
endif()

file(GLOB_RECURSE ALL_INCLUDES "${CMAKE_SOURCE_DIR}/src/*.hpp")

list(SORT INNOEXTRACT_SOURCES)
list(SORT ALL_INCLUDES)


# Prepare generated files

include_directories(src ${CMAKE_CURRENT_BINARY_DIR})

configure_file("src/configure.hpp.in" "configure.hpp")

set(VERSION_FILE "${CMAKE_BINARY_DIR}/release.cpp")
version_file("src/release.cpp.in" "${VERSION_FILE}" "VERSION" ".git")
list(APPEND INNOEXTRACT_SOURCES "${VERSION_FILE}")


# Main targets

add_executable(innoextract ${INNOEXTRACT_SOURCES} ${ALL_INCLUDES})
target_link_libraries(innoextract ${LIBRARIES})

install(TARGETS innoextract RUNTIME DESTINATION bin)

install(FILES doc/innoextract.1 DESTINATION ${MAN_DIR}/man1 OPTIONAL)


# Additional targets.

add_style_check_target(style "${INNOEXTRACT_SOURCES}" "${ALL_INCLUDES}")

add_doxygen_target(doc "doc/Doxyfile.in" "VERSION" ".git" "${CMAKE_BINARY_DIR}/doc")


# Print a configuration summary

message("")
message("Configuration:")
message(" - Build type: ${CMAKE_BUILD_TYPE}")
if(HAVE_LZMA)
	message(" - LZMA decompression: enabled")
else()
	message(" - LZMA decompression: disabled")
endif()
message("")
