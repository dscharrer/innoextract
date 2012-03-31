project(InnoExtract)

cmake_minimum_required(VERSION 2.8)

# For custom cmake modules.
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

option(USE_LZMA "Build lzma decompression support." ON)
option(DEBUG_EXTRA "Expensive debug options" OFF)

set(MAN_DIR "share/man" CACHE STRING "Install location for man pages (relative to prefix).")
mark_as_advanced(MAN_DIR)

include(CompileCheck)
include(VersionString)
include(CheckSymbolExists)
include(BuildType)
include(StyleCheck)
include(TestBigEndian)

# Force re-checking libraries if the compiler or compiler flags change.
if((NOT LAST_CMAKE_CXX_FLAGS STREQUAL CMAKE_CXX_FLAGS)
   OR (NOT LAST_CMAKE_CXX_COMPILER STREQUAL CMAKE_CXX_COMPILER))
	force_recheck_library(LZMA)
	force_recheck_library(Boost)
	unset(Boost_INCLUDE_DIR CACHE)
	set(LAST_CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}" CACHE INTERNAL "The last C++ compiler flags.")
	set(LAST_CMAKE_CXX_COMPILER "${CMAKE_CXX_COMPILER}" CACHE INTERNAL "The last C++ compiler.")
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

add_cxxflag("-Wall")
add_cxxflag("-Wextra")
add_cxxflag("-Wformat=2")
add_cxxflag("-Wundef")
add_cxxflag("-Wpointer-arith")
add_cxxflag("-Wcast-qual")
add_cxxflag("-Woverloaded-virtual")
add_cxxflag("-Wlogical-op")
add_cxxflag("-Wliteral-conversion")
add_cxxflag("-Wshift-overflow")
add_cxxflag("-Woverflow")
add_cxxflag("-Wbool-conversions")
add_cxxflag("-Wconversion")
add_cxxflag("-Wsign-conversion")
add_cxxflag("-Wmissing-declarations")
add_cxxflag("-Wredundant-decls")

if(DEBUG_EXTRA)
	add_cxxflag("-ftrapv") # to add checks for (undefined) signed integer overflow
	add_cxxflag("-fbounds-checking")
	add_cxxflag("-fcatch-undefined-behavior")
	add_cxxflag("-Wstrict-aliasing=1")
else()
	# -Wuninitialized causes too many false positives - thanks very much, gcc
	add_cxxflag("-Wno-uninitialized")
	# (clang only) Conflicts with using const variables for configuration.
	add_cxxflag("-Wno-constant-logical-operand")
	add_cxxflag("-Wno-unneeded-internal-declaration")
	add_cxxflag("-Wno-unused-function")
endif()

# Because i'm lazy
add_cxxflag("-Wl,--as-needed")

check_symbol_exists(isatty "unistd.h" HAVE_ISATTY)
check_symbol_exists(ioctl "sys/ioctl.h" HAVE_IOCTL)

test_big_endian(HAVE_BIG_ENDIAN)

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

include_directories(src ${CMAKE_CURRENT_BINARY_DIR})

configure_file("src/configure.hpp.in" "configure.hpp")

set(VERSION_FILE "${CMAKE_BINARY_DIR}/version.cpp")
version_file("src/version.cpp.in" "${VERSION_FILE}" "VERSION" ".git")
list(APPEND INNOEXTRACT_SOURCES "${VERSION_FILE}")

add_executable(innoextract ${INNOEXTRACT_SOURCES} ${ALL_INCLUDES})
target_link_libraries(innoextract ${LIBRARIES})

install(TARGETS innoextract RUNTIME DESTINATION bin)

install(FILES doc/innoextract.1 DESTINATION ${MAN_DIR}/man1 OPTIONAL)

# Additional targets.

add_style_check_target(style "${INNOEXTRACT_SOURCES}" "${ALL_INCLUDES}")
