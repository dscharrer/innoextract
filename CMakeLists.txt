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
	src/BlockReader.cpp
	src/ChunkFilter.cpp
	src/ExeReader.cpp
	src/InnoExtract.cpp
	src/LanguageEntry.cpp
	src/LoadingUtils.cpp
	src/LzmaFilter.cpp
	src/Output.cpp
	src/SetupHeader.cpp
	src/SetupLoader.cpp
	src/Utils.cpp
	src/Version.cpp
)

add_executable(innoextract ${INNOEXTRACT_SOURCES})
target_link_libraries(innoextract ${LIBRARIES})
