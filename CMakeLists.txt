cmake_minimum_required(VERSION 2.6)

find_package(ZLIB)
find_package(BZip2)

unset(LIBRARIES)

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

list(APPEND CMAKE_C_FLAGS "-ggdb -O3")
list(APPEND CMAKE_CXX_FLAGS "-ggdb -O3")

set(LIBLZMADEC_SOURCES
	liblzmadec/main.c
)

set(INNOEXTRACT_SOURCES
	ExeReader.cpp
	InnoExtract.cpp
	SetupLoader.cpp
	Utils.cpp
)

set(UNDEFLATE_SOURCES UnDeflate.cpp)

add_library(lzmadec STATIC ${LIBLZMADEC_SOURCES})
list(APPEND LIBRARIES lzmadec)

add_executable(innoextract ${INNOEXTRACT_SOURCES})
target_link_libraries(innoextract ${LIBRARIES})

add_executable(undeflate ${UNDEFLATE_SOURCES})
target_link_libraries(undeflate ${LIBRARIES} -llzma)
