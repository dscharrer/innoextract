
# Copyright (C) 2011-2013 Daniel Scharrer
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

if((NOT DEFINED INPUT) OR (NOT DEFINED OUTPUT) OR (NOT DEFINED VERSION_SOURCES) OR (NOT DEFINED GIT_DIR))
	message(SEND_ERROR "Invalid arguments.")
endif()

# configure_file doesn't handle newlines correctly - pre-escape variables
function(escape_var VAR)
	# Escape the escape character and quotes
	string(REGEX REPLACE "([\\\\\"])" "\\\\\\1" escaped "${${VAR}}")
	# Pull newlines out of string
	string(REGEX REPLACE "\n" "\\\\n\"\n\t\"" escaped "${escaped}")
	set(${VAR} "${escaped}" PARENT_SCOPE)
endfunction(escape_var)

set(var "")
foreach(arg IN LISTS VERSION_SOURCES)
	
	if(var STREQUAL "")
		set(var ${arg})
	else()
		
		file(READ "${arg}" ${var})
		string(STRIP "${${var}}" ${var})
		string(REGEX REPLACE "\r\n" "\n" ${var} "${${var}}")
		string(REGEX REPLACE "\r" "\n" ${var} "${${var}}")
		
		# Split the version file into lines.
		string(REGEX MATCHALL "[^\r\n]+" lines "${${var}}")
		set(${var}_COUNT 0)
		foreach(line IN LISTS lines)
			
			set(${var}_${${var}_COUNT} "${line}")
			escape_var(${var}_${${var}_COUNT})
			
			# Find the first and last spaces
			string(STRIP "${line}" line)
			string(LENGTH "${line}" line_length)
			set(first_space -1)
			set(last_space ${line_length})
			foreach(i RANGE ${line_length})
				if(${i} LESS ${line_length})
					string(SUBSTRING "${line}" ${i} 1 line_char)
					if(line_char STREQUAL " ")
						set(last_space ${i})
						if(first_space EQUAL -1)
							set(first_space ${i})
						endif()
					endif()
				endif()
			endforeach()
			
			if(${first_space} GREATER -1)
				
				# Get everything before the first space
				string(SUBSTRING "${line}" 0 ${first_space} line_name)
				string(STRIP "${line_name}" ${var}_${${var}_COUNT}_SHORTNAME)
				escape_var(${var}_${${var}_COUNT}_SHORTNAME)
				
				# Get everything after the first space
				math(EXPR num_length "${line_length} - ${first_space}")
				string(SUBSTRING "${line}" ${first_space} ${num_length} line_num)
				string(STRIP "${line_num}" ${var}_${${var}_COUNT}_STRING)
				escape_var(${var}_${${var}_COUNT}_STRING)
				
			endif()
			
			# Get everything before the last space
			string(SUBSTRING "${line}" 0 ${last_space} line_name)
			string(STRIP "${line_name}" ${var}_${${var}_COUNT}_NAME)
			escape_var(${var}_${${var}_COUNT}_NAME)
			
			# Get everything after the last space
			if(${last_space} LESS ${line_length})
				math(EXPR num_length "${line_length} - ${last_space}")
				string(SUBSTRING "${line}" ${last_space} ${num_length} line_num)
				string(STRIP "${line_num}" ${var}_${${var}_COUNT}_NUMBER)
				escape_var(${var}_${${var}_COUNT}_NUMBER)
			endif()
			
			math(EXPR ${var}_COUNT "${${var}_COUNT} + 1")
		endforeach()
		
		string(REGEX REPLACE "\n\n.*$" "" ${var}_HEAD "${${var}}")
		string(STRIP "${${var}_HEAD}" ${var}_HEAD)
		string(REGEX MATCH "\n\n.*" ${var}_TAIL "${${var}}")
		string(STRIP "${${var}_TAIL}" ${var}_TAIL)
		
		escape_var(${var})
		escape_var(${var}_HEAD)
		escape_var(${var}_TAIL)
		
		set(var "")
	endif()
	
endforeach()

# Check for a git directory and fill in the git commit hash if one exists.
unset(GIT_COMMIT)
if(EXISTS "${GIT_DIR}")
	
	unset(git_head)
	
	if(GIT_COMMAND)
		execute_process(
			COMMAND "${GIT_COMMAND}" "--git-dir=${GIT_DIR}" "rev-parse" "HEAD"
			RESULT_VARIABLE result
			OUTPUT_VARIABLE git_head
		)
		if(NOT "${result}" EQUAL 0)
			unset(git_head)
		endif()
	endif()
	
	if(NOT git_head AND EXISTS "${GIT_DIR}/HEAD")
		
		file(READ "${GIT_DIR}/HEAD" git_head)
		
		if("${git_head}" MATCHES "^[ \t\r\n]*ref\\:(.*)$")
			
			# Remove the first for characters from git_head to get git_ref.
			# We can't use a length of -1 for string(SUBSTRING) as cmake < 2.8.5 doesn't support it.
			string(LENGTH "${git_head}" git_head_length)
			math(EXPR git_ref_length "${git_head_length} - 4")
			string(SUBSTRING "${git_head}" 4 ${git_ref_length} git_ref)
			string(STRIP "${git_ref}" git_ref)
			
			unset(git_head)
			if(EXISTS "${GIT_DIR}/${git_ref}")
				file(READ "${GIT_DIR}/${git_ref}" git_head)
			elseif(EXISTS "${GIT_DIR}/packed-refs")
				file(READ "${GIT_DIR}/packed-refs" git_refs)
				string(REGEX REPLACE "[^0-9A-Za-z]" "\\\\\\0" git_ref "${git_ref}")
				string(REGEX MATCH "[^\r\n]* ${git_ref}( [^\r\n])?" git_head "${git_refs}")
			endif()
			
		endif()
		
	endif()
	
	# Create variables for all prefixes of the git comit ID.
	string(REGEX MATCH "[0-9A-Za-z]+" git_commit "${git_head}")
	string(LENGTH "${git_commit}" git_commit_length)
	if(NOT ${git_commit_length} LESS 40)
		string(TOLOWER "${git_commit}" GIT_COMMIT)
		foreach(i RANGE 20)
			string(SUBSTRING "${GIT_COMMIT}" 0 ${i} GIT_COMMIT_PREFIX_${i})
			set(GIT_SUFFIX_${i} " + ${GIT_COMMIT_PREFIX_${i}}")
		endforeach()
	else()
		message(WARNING "Git repository detected, but could not determine HEAD")
	endif()
	
endif()

configure_file("${INPUT}" "${OUTPUT}")
