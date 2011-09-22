cmake_minimum_required(VERSION 2.6)

find_package(ZLIB)
find_package(BZip2)

unset(LIBRARIES)

find_package(Boost REQUIRED COMPONENTS iostreams)
list(APPEND LIBRARIES "${Boost_LIBRARIES}")
link_directories("${Boost_LIBRARY_DIRS}")
include_directories(SYSTEM "${Boost_INCLUDE_DIR}")

if(ZLIB_FOUND)
	include_directories(SYSTEM "${ZLIB_INCLUDE_DIR}")
	list(APPEND LIBRARIES "${ZLIB_LIBRARIES}")
	add_definitions(-DHAVE_ZLIB)
endif()

if(BZIP2_FOUND)
	include_directories(SYSTEM "${BZIP2_INCLUDE_DIR}")
	list(APPEND LIBRARIES "${BZIP2_LIBRARIES}")
	add_definitions(-DHAVE_BZIP2)
endif()

list(APPEND LIBRARIES -llzma)

list(APPEND CMAKE_C_FLAGS "-ggdb -O3 -march=native")
list(APPEND CMAKE_CXX_FLAGS "-ggdb -O3 -march=native")

set(LIBLZMADEC_SOURCES
	liblzmadec/main.c
)

set(INNOEXTRACT_SOURCES
	BlockReader.cpp
	ChunkFilter.cpp
	ExeReader.cpp
	InnoExtract.cpp
	LzmaFilter.cpp
	SetupLoader.cpp
	Utils.cpp
)

set(UNDEFLATE_SOURCES UnDeflate.cpp)

add_library(lzmadec STATIC ${LIBLZMADEC_SOURCES})
list(APPEND LIBRARIES lzmadec)

add_executable(innoextract ${INNOEXTRACT_SOURCES})
target_link_libraries(innoextract ${LIBRARIES})

add_executable(undeflate ${UNDEFLATE_SOURCES})
target_link_libraries(undeflate ${LIBRARIES})
