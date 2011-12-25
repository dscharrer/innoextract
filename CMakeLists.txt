cmake_minimum_required(VERSION 2.6)

# For custom cmake modules.
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

include(CompileCheck)
include(VersionString)
include(CheckSymbolExists)

# Force re-checking libraries if the compiler or compiler flags change.
if((NOT LAST_CMAKE_CXX_FLAGS STREQUAL CMAKE_CXX_FLAGS)
   OR (NOT LAST_CMAKE_CXX_COMPILER STREQUAL CMAKE_CXX_COMPILER))
	force_recheck_library(Boost)
	unset(Boost_INCLUDE_DIR CACHE)
	set(LAST_CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}" CACHE INTERNAL "The last C++ compiler flags.")
	set(LAST_CMAKE_CXX_COMPILER "${CMAKE_CXX_COMPILER}" CACHE INTERNAL "The last C++ compiler.")
endif()


unset(LIBRARIES)

find_package(Boost REQUIRED COMPONENTS iostreams filesystem date_time system)
check_link_library(Boost Boost_LIBRARIES)
list(APPEND LIBRARIES "${Boost_LIBRARIES}")
link_directories("${Boost_LIBRARY_DIRS}")
include_directories(SYSTEM "${Boost_INCLUDE_DIR}")

find_package(LZMA)
check_link_library(LZMA LZMA_LIBRARIES)
list(APPEND LIBRARIES "${LZMA_LIBRARIES}")
include_directories(SYSTEM "${LZMA_INCLUDE_DIR}")

# TODO make LZMA optional

# TODO debugging-only
add_cxxflag("-ggdb")
add_cxxflag("-O3")
add_definitions(-D_DEBUG)

# TODO should probably be supplied by the user
add_cxxflag("-march=native")
add_cxxflag("-Wl,--as-needed")

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
	add_definitions(-D_GLIBCXX_DEBUG) # Runtime checks for STL containers.
	add_cxxflag("-ftrapv") # to add checks for (undefined) signed integer overflow
	add_cxxflag("-fcatch-undefined-behavior") # (clang)
	add_cxxflag("-fbounds-checking")
else()
	# -Wuninitialized causes too many false positives - thanks very much, gcc
	add_cxxflag("-Wno-uninitialized")
endif()

if(UNITY_BUILD)
	add_cxxflag("-fwhole-program")
endif()

check_symbol_exists(isatty "unistd.h" HAVE_ISATTY)
check_symbol_exists(ioctl "sys/ioctl.h" HAVE_IOCTL)

set(INNOEXTRACT_SOURCES
	
	src/cli/debug.cpp
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
	src/setup/file.cpp
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
	src/stream/lzma.cpp
	src/stream/slice.cpp
	
	src/util/console.cpp
	src/util/load.cpp
	src/util/log.cpp
	
)

file(GLOB_RECURSE ALL_INCLUDES "${CMAKE_SOURCE_DIR}/src/*.hpp")

include_directories(src ${CMAKE_CURRENT_BINARY_DIR})

configure_file("src/configure.hpp.in" "configure.hpp")

set(VERSION_FILE "${CMAKE_BINARY_DIR}/version.cpp")
version_file("src/version.cpp.in" "${VERSION_FILE}" "VERSION" ".git")
list(APPEND INNOEXTRACT_SOURCES "${VERSION_FILE}")

add_executable(innoextract ${INNOEXTRACT_SOURCES} ${ALL_INCLUDES})
target_link_libraries(innoextract ${LIBRARIES})

install(TARGETS innoextract RUNTIME DESTINATION bin)

# Additional targets.

find_package(PythonInterp)

if(PYTHONINTERP_FOUND)
	
	unset(STYLE_FILTER)
	
	# Complains about any c-style cast -> too annoying.
	set(STYLE_FILTER ${STYLE_FILTER},-readability/casting)
	
	# Insists on including evrything in the .cpp file even if it is included in the header.
	# This behaviour conflicts with orther tools.
	set(STYLE_FILTER ${STYLE_FILTER},-build/include_what_you_use)
	
	# Too many false positives and not very helpful error messages.
	set(STYLE_FILTER ${STYLE_FILTER},-build/include_order)
	
	# No thanks.
	set(STYLE_FILTER ${STYLE_FILTER},-readability/streams)
	
	# Ugh!
	set(STYLE_FILTER ${STYLE_FILTER},-whitespace/tab)
	
	# Yes it is!
	set(STYLE_FILTER ${STYLE_FILTER},-whitespace/blank_line)
	
	# Suggessts excessive indentation.
	set(STYLE_FILTER ${STYLE_FILTER},-whitespace/labels)
	
	# Don't tell me how to name my variables.
	set(STYLE_FILTER ${STYLE_FILTER},-runtime/arrays)
	
	# Why?
	set(STYLE_FILTER ${STYLE_FILTER},-whitespace/todo)
	set(STYLE_FILTER ${STYLE_FILTER},-readability/todo)
	
	# TODO add copyright notices
	set(STYLE_FILTER ${STYLE_FILTER},-legal/copyright)
	
	add_custom_target(style
		COMMAND cmake -E chdir "${CMAKE_SOURCE_DIR}" "${PYTHON_EXECUTABLE}" "${CMAKE_MODULE_PATH}/cpplint.py" "--filter=${STYLE_FILTER}" ${INNOEXTRACT_SOURCES} ${ALL_INCLUDES}
	)
	
endif()
