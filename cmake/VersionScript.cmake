
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

cmake_minimum_required(VERSION 2.8)

# CMake script that reads a VERSION file and the current git history and the calls configure_file().
# This is used by version_file() in VersionString.cmake

if((NOT DEFINED INPUT) OR (NOT DEFINED OUTPUT) OR (NOT DEFINED VERSION_FILE) OR (NOT DEFINED GIT_DIR))
	message(SEND_ERROR "Invalid arguments.")
endif()

file(READ "${VERSION_FILE}" BASE_VERSION)
string(STRIP "${BASE_VERSION}" BASE_VERSION)

# Split the version file into lines.
string(REGEX MATCHALL "[^\r\n]+" version_lines "${BASE_VERSION}")
set(BASE_VERSION_COUNT 0)
foreach(version_line IN LISTS version_lines)
	set(BASE_VERSION_${BASE_VERSION_COUNT} "${version_line}")
	math(EXPR BASE_VERSION_COUNT "${BASE_VERSION_COUNT} + 1")
endforeach()

# Check for a git directory and fill in the git commit hash if one exists.
unset(GIT_COMMIT)
if(EXISTS "${GIT_DIR}")
	
	file(READ "${GIT_DIR}/HEAD" git_head)
	string(STRIP "${git_head}" git_head)
	
	if("${git_head}" MATCHES "^ref\\:")
		
		# Remove the first for characters from git_head to get git_ref.
		# We can't use a length of -1 for string(SUBSTRING) as cmake < 2.8.5 doesn't support it.
		string(LENGTH "${git_head}" git_head_length)
		math(EXPR git_ref_length "${git_head_length} - 4")
		string(SUBSTRING "${git_head}" 4 ${git_ref_length} git_ref)
		
		string(STRIP "${git_ref}" git_ref)
		
		file(READ "${GIT_DIR}/${git_ref}" git_head)
		string(STRIP "${git_head}" git_head)
	endif()
	
	string(REGEX MATCH "[0-9A-Za-z]+" GIT_COMMIT "${git_head}")
	
	# Create variables for all prefixes of the git comit ID.
	if(GIT_COMMIT)
		string(TOLOWER "${GIT_COMMIT}" GIT_COMMIT)
		string(LENGTH "${GIT_COMMIT}" git_commit_length)
		foreach(i RANGE "${git_commit_length}")
			string(SUBSTRING "${GIT_COMMIT}" 0 ${i} GIT_COMMIT_PREFIX_${i})
		endforeach()
	endif()
	
endif()

configure_file("${INPUT}" "${OUTPUT}" ESCAPE_QUOTES)
