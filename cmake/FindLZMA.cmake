
# Try to find the LZMA library and include path for lzma.h from xz-utils.
# Once done this will define
#
# LZMA_FOUND
# LZMA_INCLUDE_DIR - where to find lzma.h
# LZMA_LIBRARIES - liblzma.so

find_path(LZMA_INCLUDE_DIR lzma.h DOC "The directory where lzma.h resides")
find_library(LZMA_LIBRARY lzma DOC "The LZMA library")

mark_as_advanced(LZMA_INCLUDE_DIR)
mark_as_advanced(LZMA_LIBRARY)

# handle the QUIETLY and REQUIRED arguments and set LZMA_FOUND to TRUE if 
# all listed variables are TRUE
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(LZMA DEFAULT_MSG LZMA_LIBRARY LZMA_INCLUDE_DIR)

if(LZMA_FOUND)
	set(LZMA_LIBRARIES ${LZMA_LIBRARY})
endif(LZMA_FOUND)
