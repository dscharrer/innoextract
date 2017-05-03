
# Copyright (C) 2011-2012 Daniel Scharrer
#
# This software is provided 'as-is', without any express or implied
# warranty.  In no event will the author(s) be held liable for any damages
# arising from the use of this software.
#
# Permission is granted to anyone to use this software for any purpose,
# including commercial applications, and to alter it and redistribute it
# freely, subject to the following restrictions:
#
# 1. The origin of this software must not be misrepresented; you must not
#    claim that you wrote the original software. If you use this software
#    in a product, an acknowledgment in the product documentation would be
#    appreciated but is not required.
# 2. Altered source versions must be plainly marked as such, and must not be
#    misrepresented as being the original software.
# 3. This notice may not be removed or altered from any source distribution.

find_package(Doxygen)

include(VersionString)

# Add a target that runs Doxygen on a configured Doxyfile
#
# Parameters:
# - TARGET_NAME the name of the target to add
# - DOXYFILE_IN the raw Doxyfile
# - VERSION_FILE VERSION file to be used by version_file()
# - GIT_DIR .git directory to be used by version_file()
# - OUT_DIR Doxygen output directory
#
# For the exact syntax of config options in DOXYFILE_IN see the documentation of the
# configure_file() cmake command.
#
# Available variables are those provided by version_file() as well as
# DOXYGEN_OUTPUT_DIR, which is set to OUT_DIR.
#
function(add_doxygen_target TARGET_NAME DOXYFILE_IN VERSION_FILE GIT_DIR OUT_DIR)
	
	if(NOT DOXYGEN_EXECUTABLE)
		return()
	endif()
	
	set(doxyfile "${PROJECT_BINARY_DIR}/Doxyfile.${TARGET_NAME}")
	set(defines "-DDOXYGEN_OUTPUT_DIR=${OUT_DIR}")
	
	version_file("${DOXYFILE_IN}" "${doxyfile}" "${VERSION_FILE}" "${GIT_DIR}" "${defines}")
	
	add_custom_target(${TARGET_NAME}
		COMMAND "${CMAKE_COMMAND}" -E make_directory "${OUT_DIR}"
		COMMAND ${DOXYGEN_EXECUTABLE} "${doxyfile}"
		DEPENDS "${doxyfile}"
		WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
		COMMENT "Building doxygen documentation."
		VERBATIM
		SOURCES "${DOXYFILE_IN}"
	)
	
endfunction(add_doxygen_target)
