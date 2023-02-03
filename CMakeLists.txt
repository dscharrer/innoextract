cmake_minimum_required(VERSION 2.8...3.19)

if(CMAKE_VERSION VERSION_LESS 3.12)
	cmake_policy(VERSION ${CMAKE_VERSION})
endif()

project(innoextract)

# set(DEBUG 1)

unset(LIBRARIES)
set(INNOEXTRACT_HAVE_LZMA 1)

if(NOT DEFINED ENV{ZIPDIR})
	message(FATAL_ERROR "ZIPDIR not set in environment, exiting.")
endif()


if(USE_EMBOOST)
	if(NOT DEFINED ENV{EMBOOSTDIR})
		message(FATAL_ERROR "EMBOOSTDIR not set in environment, exiting.")
	endif()

	SET(EMBOOSTDIR $ENV{EMBOOSTDIR})
	set(EMBDIR "${EMBOOSTDIR}/stage/lib")
	set(EMHEADERS "${EMBOOSTDIR}/boost")

	set(EMLIBS 
		${EMBDIR}/libboost_date_time.bc
		${EMBDIR}/libboost_filesystem.bc
		${EMBDIR}/libboost_iostreams.bc
		${EMBDIR}/libboost_program_options.bc
		${EMBDIR}/libboost_system.bc
		${EMBDIR}/libboost_zlib.bc
		${EMBDIR}/libboost_bzip2.bc

	)
	list(APPEND LIBRARIES ${EMLIBS})
endif()

if(NOT USE_EMBOOST)
	find_package(Boost REQUIRED COMPONENTS
		iostreams
		filesystem
		date_time
		system
		program_options
	)

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
endif()

# Set compiler flags
add_compile_options(-std=c++1y -Wno-deprecated-declarations -sUSE_ZLIB=1 -O3 -fexceptions)

add_link_options(
-sINITIAL_MEMORY=1024MB -sSTACK_SIZE=512MB -sALLOW_MEMORY_GROWTH 
-sEXPORTED_RUNTIME_METHODS=ccall,cwrap -sINVOKE_RUN=0 -sASYNCIFY -sASYNCIFY_STACK_SIZE=65565
--pre-js ../src/pre.js -sNO_DISABLE_EXCEPTION_CATCHING )

add_compile_options("-fvisibility=hidden")
add_compile_options("-fvisibility-inlines-hidden")

if($ENV{PORTAGE_REPO_NAME} MATCHES "gentoo")
	# Meh
	unset(LIBRARIES)
endif()

# All sources:
set(INNOEXTRACT_SOURCES
	src/release.hpp
	src/release.cpp
	
	src/cli/debug.hpp
	src/cli/extract.hpp
	src/cli/extract.cpp
	src/cli/gog.hpp
	src/cli/gog.cpp
	src/cli/goggalaxy.hpp
	src/cli/goggalaxy.cpp
	src/cli/main.cpp
	
	src/crypto/adler32.hpp
	src/crypto/adler32.cpp
	# src/crypto/arc4.hpp if INNOEXTRACT_HAVE_ARC4
	# src/crypto/arc4.cpp if INNOEXTRACT_HAVE_ARC4
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
	src/emjs.cpp	
)

if(${DEBUG})
	add_compile_options(-DDEBUG)
	list(APPEND INNOEXTRACT_SOURCES src/cli/debug.cpp)
endif()


if(USE_LZMA)
	if(NOT DEFINED ENV{LZMADIR})
		message(FATAL_ERROR "LZMADIR not set in environment, exiting.")
	endif()

	set(LZMAHEADERS $ENV{LZMADIR}/api)
	set(LZMALIBS $ENV{LZMADIR}/.libs/liblzma.a )
	
	list(APPEND LIBRARIES ${LZMALIBS})
	list(APPEND INNOEXTRACT_SOURCES
		src/stream/lzma.hpp
		src/stream/lzma.cpp
	)
endif()

# libzip
set(ZIPHEADERS $ENV{ZIPDIR}/lib/)
set(ZIPLIBS	$ENV{ZIPDIR}/lib/libzip.a )

list(APPEND LIBRARIES ${ZIPLIBS})
# create_source_groups(ALL_INNOEXTRACT_SOURCES)


# Prepare generated files

include_directories(src ${CMAKE_CURRENT_BINARY_DIR} ${EMHEADERS} ${LZMAHEADERS} ${ZIPHEADERS})

configure_file("src/configure.hpp.in" "configure.hpp")
configure_file("src/index.html" "${CMAKE_CURRENT_BINARY_DIR}/index.html" COPYONLY)

# Main targets

set(CMAKE_EXECUTABLE_SUFFIX ".html")
add_executable(innoextract ${INNOEXTRACT_SOURCES})
target_link_libraries(innoextract ${LIBRARIES})

install(TARGETS innoextract RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

install(FILES ${MAN_FILE} DESTINATION ${CMAKE_INSTALL_MANDIR}/man1 OPTIONAL)

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
message("ARC4 decryption" FIRST
	INNOEXTRACT_HAVE_ARC4 "enabled"
	1                     "disabled"
)
message("LZMA decompression" FIRST
	INNOEXTRACT_HAVE_LZMA "enabled"
	1                     "disabled"
)
if(INNOEXTRACT_HAVE_DYNAMIC_UTIMENSAT)
	set(time_prefix "nanoseconds if supported, ")
	set(time_suffix " otherwise")
endif()
message("File time precision" FIRST
	INNOEXTRACT_HAVE_UTIMENSAT_d "nanoseconds"
	WIN32                        "100-nanoseconds"
	INNOEXTRACT_HAVE_UTIMES      "${time_prefix}microseconds${time_suffix}"
	1                            "${time_prefix}seconds${time_suffix}"
)
message("Charset conversion"
	INNOEXTRACT_HAVE_ICONV        "iconv"
	INNOEXTRACT_HAVE_WIN32_CONV   "Win32"
	1                             "builtin"
)

message("LIBRARIES:" ${LIBRARIES})

message("")