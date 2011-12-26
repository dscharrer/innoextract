
# Copyright (C) 2011 Daniel Scharrer
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

# Create a rule to generate a version string at compile time.
#
# SRC is processed using the configure_file() cmake command
# at build to produce DST with the following variable available:
#
# - BASE_VERSION: The contents of the file specified by VERSION_FILE.
# - GIT_COMMIT: The current git commit. This variable is not defined if there is no GIT_DIR directory.
# - SHORT_GIT_COMMIT: The first 10 characters of the git commit.
# For the exact syntax of SRC see the documentation of the configure_file() cmake command.
#
# The version file is regenerated whenever VERSION_FILE or the current commit changes.
function(version_file SRC DST VERSION_FILE GIT_DIR)
	
	get_filename_component(ABS_SRC "${SRC}" ABSOLUTE)
	get_filename_component(ABS_DST "${DST}" ABSOLUTE)
	get_filename_component(ABS_VERSION_FILE "${VERSION_FILE}" ABSOLUTE)
	get_filename_component(ABS_GIT_DIR "${GIT_DIR}" ABSOLUTE)
	
	add_custom_command(
		OUTPUT
			"${DST}"
		COMMAND
			${CMAKE_COMMAND}
			"-DINPUT=${ABS_SRC}"
			"-DOUTPUT=${ABS_DST}"
			"-DVERSION_FILE=${ABS_VERSION_FILE}"
			"-DGIT_DIR=${ABS_GIT_DIR}"
			-P "${CMAKE_MODULE_PATH}/VersionScript.cmake"
		MAIN_DEPENDENCY
			"${SRC}"
		DEPENDS
			"${GIT_DIR}/HEAD"
			"${GIT_DIR}/logs/HEAD"
			"${VERSION_FILE}"
			"${CMAKE_MODULE_PATH}/VersionScript.cmake"
		COMMENT ""
		VERBATIM
	)
	
endfunction(version_file)
