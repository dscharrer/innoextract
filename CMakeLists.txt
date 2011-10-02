cmake_minimum_required(VERSION 2.6)


unset(LIBRARIES)

find_package(Boost REQUIRED COMPONENTS iostreams)
list(APPEND LIBRARIES "${Boost_LIBRARIES}")
link_directories("${Boost_LIBRARY_DIRS}")
include_directories(SYSTEM "${Boost_INCLUDE_DIR}")

#find_package(ZLIB)
#if(ZLIB_FOUND)
#	include_directories(SYSTEM "${ZLIB_INCLUDE_DIR}")
#	list(APPEND LIBRARIES "${ZLIB_LIBRARIES}")
#	add_definitions(-DHAVE_ZLIB)
#endif()

#find_package(BZip2)
#if(BZIP2_FOUND)
#	include_directories(SYSTEM "${BZIP2_INCLUDE_DIR}")
#	list(APPEND LIBRARIES "${BZIP2_LIBRARIES}")
#	add_definitions(-DHAVE_BZIP2)
#endif()

list(APPEND LIBRARIES -llzma)

list(APPEND CMAKE_CXX_FLAGS "-ggdb -O3 -march=native -Wl,--as-needed")

set(INNOEXTRACT_SOURCES
	
	src/loader/ExeReader.cpp
	src/loader/SetupLoader.cpp
	
	src/setup/CustomMessageEntry.cpp
	src/setup/DirectoryEntry.cpp
	src/setup/FileEntry.cpp
	src/setup/IconEntry.cpp
	src/setup/IniEntry.cpp
	src/setup/LanguageEntry.cpp
	src/setup/PermissionEntry.cpp
	src/setup/RegistryEntry.cpp
	src/setup/SetupComponentEntry.cpp
	src/setup/SetupCondition.cpp
	src/setup/SetupHeader.cpp
	src/setup/SetupTaskEntry.cpp
	src/setup/SetupTypeEntry.cpp
	src/setup/Version.cpp
	src/setup/WindowsVersion.cpp
	
	src/stream/BlockReader.cpp
	src/stream/ChunkFilter.cpp
	src/stream/Lzma1Filter.cpp
	
	src/util/LoadingUtils.cpp
	src/util/Output.cpp
	src/util/Utils.cpp
	
	src/InnoExtract.cpp
)

include_directories(src)

add_executable(innoextract ${INNOEXTRACT_SOURCES})
target_link_libraries(innoextract ${LIBRARIES})
