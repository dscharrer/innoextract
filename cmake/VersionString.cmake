
# Copyright (C) 2011-2020 Daniel Scharrer
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

get_filename_component(VERSION_SCRIPT_DIR "${CMAKE_CURRENT_LIST_FILE}" PATH)
set(VERSION_STRING_SCRIPT "${VERSION_SCRIPT_DIR}/VersionScript.cmake")

# Create a rule to generate a version string at compile time.
#
# An optional fifth argument can be used to add additional cmake defines.
#
# SRC is processed using the configure_file() cmake command
# at build to produce DST with the following variable available:
#
# VERSION_SOURCES:
#  List of (${var} ${file}) pairs.
#
# for each variable ${var}
# - ${var}: The contents of the associated file
# - ${var}_COUNT: Number of lines in the associated file
# - For each line ${i}:
#   - ${var}_${i}: The ${i}-th line of the associated file
#    - ${var}_${i}_PREFIX: The first component in the line
#    - ${var}_${i}_LINE: Everything except the first component of the line
#    - ${var}_${i}_NAME: Everything except the last component of the line
#    - ${var}_${i}_STRING: The last component (excluding optional suffix) of the line
#    - ${var}_${i}_SUFFIX: Suffix (seperated by " + ") of the line
#    - ${var}_${i}_MAJOR: First version component in ${var}_${i}_STRING
#    - ${var}_${i}_MINOR: Second version component in ${var}_${i}_STRING
#    - ${var}_${i}_PATCH: Third version component in ${var}_${i}_STRING
#    - ${var}_${i}_BUILD: Fourth version component in ${var}_${i}_STRING
#    - ${var}_${i}_NUMBER: Reassembled verion components
#    - ${var}_${i}_PRERELEASE: If the version indicates a prerelease build
#    - ${var}_${i}_PRIVATE: If the version indicates a private build
# - ${var}_HEAD: The first paragraph of the associated file
# - ${var}_TAIL: The remaining paragraphs of the associated file
#
# - GIT_COMMIT: The current git commit. (not defined if there is no GIT_DIR directory)
# - GIT_COMMIT_PREFIX_${i}: The first ${i} characters of GIT_COMMIT (i=0..39)
# For the exact syntax of SRC see the documentation of the configure_file() cmake command.
# The version file is regenerated whenever VERSION_FILE or the current commit changes.
function(version_file SRC DST VERSION_SOURCES GIT_DIR)
	
	set(MODE_VARIABLE 0)
	set(MODE_FILE 1)
	
	set(mode ${MODE_VARIABLE})
	
	set(args)
	set(dependencies "${VERSION_STRING_SCRIPT}")
	
	foreach(arg IN LISTS VERSION_SOURCES)
		
		if(mode EQUAL MODE_VARIABLE)
			set(mode ${MODE_FILE})
		else()
			get_filename_component(arg "${arg}" ABSOLUTE)
			list(APPEND dependencies ${arg})
			set(mode ${MODE_VARIABLE})
		endif()
		
		list(APPEND args ${arg})
		
	endforeach()
	
	get_filename_component(abs_src "${SRC}" ABSOLUTE)
	get_filename_component(abs_dst "${DST}" ABSOLUTE)
	get_filename_component(abs_git_dir "${GIT_DIR}" ABSOLUTE)
	
	set(defines)
	if(ARGC GREATER 4)
		set(defines ${ARGV4})
	endif()
	
	if(EXISTS "${abs_git_dir}")
		find_program(GIT_COMMAND git)
		if(GIT_COMMAND)
			list(APPEND dependencies "${GIT_COMMAND}")
		endif()
		if(EXISTS "${abs_git_dir}/HEAD")
			list(APPEND dependencies "${abs_git_dir}/HEAD")
		endif()
		if(EXISTS "${abs_git_dir}/packed-refs")
			list(APPEND dependencies "${abs_git_dir}/packed-refs")
		endif()
		if(EXISTS "${abs_git_dir}/logs/HEAD")
			list(APPEND dependencies "${abs_git_dir}/logs/HEAD")
		endif()
	else()
		set(abs_git_dir "")
	endif()
	
	add_custom_command(
		OUTPUT
			"${abs_dst}"
		COMMAND
			${CMAKE_COMMAND}
			"-DINPUT=${abs_src}"
			"-DOUTPUT=${abs_dst}"
			"-DVERSION_SOURCES=${args}"
			"-DGIT_DIR=${abs_git_dir}"
			"-DGIT_COMMAND=${GIT_COMMAND}"
			${defines}
			-P "${VERSION_STRING_SCRIPT}"
		COMMAND
			${CMAKE_COMMAND} -E touch "${abs_dst}"
		MAIN_DEPENDENCY
			"${abs_src}"
		DEPENDS
			${dependencies}
		COMMENT ""
		VERBATIM
	)
	
endfunction()

macro(_version_escape var string)
	set(${var} "${string}")
endmacro()

macro(_define_version_var_nostrip suffix contents)
	_version_escape(${var}_${suffix} "${${contents}}")
	list(APPEND variables ${var}_${suffix})
endmacro()

macro(_define_version_var suffix contents)
	string(STRIP "${${contents}}" tmp)
	_define_version_var_nostrip(${suffix} tmp)
endmacro()

macro(_define_version_line_var_nostrip suffix contents)
	_define_version_var_nostrip(${i}_${suffix} ${contents})
	if(line_name)
		set(${line_name}_${suffix} "${${var}_${i}_${suffix}}")
		list(APPEND variables ${line_name}_${suffix})
	endif()
endmacro()

macro(_define_version_line_var suffix contents)
	string(STRIP "${${contents}}" tmp)
	_define_version_line_var_nostrip(${suffix} tmp)
endmacro()

function(parse_version_file names file)
	
	list(GET names 0 var)
	
	list(LENGTH names names_count)
	
	set(variables)
	
	file(READ "${file}" contents)
	string(STRIP "${contents}" contents)
	string(REGEX REPLACE "\r\n" "\n" contents "${contents}")
	string(REGEX REPLACE "\r" "\n" contents "${contents}")
	_version_escape(${var} "${contents}")
	list(APPEND variables ${var})
	
	# Split the version file into lines.
	string(REGEX MATCHALL "[^\r\n]+" lines "${contents}")
	set(i 0)
	foreach(line IN LISTS lines)
		
		if(i LESS names_count)
			list(GET names ${i} line_name)
		else()
			set(line_name)
		endif()
		
		_define_version_var(${i} line)
		
		if(line MATCHES "^([^ ]*) (.*)$")
			set(prefix "${CMAKE_MATCH_1}")
			set(notprefix "${CMAKE_MATCH_2}")
			_define_version_line_var(PREFIX prefix)
			_define_version_line_var(LINE notprefix)
		else()
			_define_version_line_var(PREFIX line)
		endif()
		
		if(line MATCHES "^(.*[^+] )?([^ ]+)( \\+ [^ ]+)?$")
			
			set(name "${CMAKE_MATCH_1}")
			set(string "${CMAKE_MATCH_2}")
			set(suffix "${CMAKE_MATCH_3}")
			
			_define_version_line_var(NAME name)
			_define_version_line_var(STRING string)
			_define_version_line_var_nostrip(SUFFIX suffix)
			
			if(i GREATER 0 AND line_name)
				set(${line_name} "${${var}_${i}_STRING}")
				list(APPEND variables ${line_name})
			endif()
			
			if(string MATCHES "^([0-9]+)(\\.([0-9]+)(\\.([0-9]+)(\\.([0-9]+))?)?)?(.*)?$")
				
				set(major "${CMAKE_MATCH_1}")
				set(minor "${CMAKE_MATCH_3}")
				set(patch "${CMAKE_MATCH_5}")
				set(build "${CMAKE_MATCH_7}")
				set(release "${CMAKE_MATCH_8}")
				
				set(error 0)
				set(newpatch)
				set(newbuild)
				set(prerelease 0)
				set(private 0)
				if(release MATCHES "^\\-dev\\-([2-9][0-9][0-9][0-9]+)\\-([0-9][0-9])\\-([0-9][0-9])$")
					set(prerelease 1)
					set(newpatch "${CMAKE_MATCH_1}")
					set(newbuild "${CMAKE_MATCH_2}${CMAKE_MATCH_3}")
				elseif(release MATCHES "^\\-rc([0-9]+)$")
					set(prerelease 1)
					set(newpatch "9999")
					set(newbuild "${CMAKE_MATCH_1}")
				elseif(release MATCHES "^\\-r([0-9]+)$")
					if(build STREQUAL "")
						set(build "${CMAKE_MATCH_1}")
					else()
						set(error 1)
					endif()
				elseif(release OR suffix)
					set(prerelease 1)
					set(private 1)
					set(newpatch 9999)
					set(newbuild 9999)
				endif()
				
				foreach(component IN ITEMS major minor patch build newpatch newbuild)
					string(REGEX REPLACE "^0+" "" ${component} "${${component}}")
					if(NOT ${component})
						set(${component} 0)
					endif()
				endforeach()
				
				if(newpatch)
					set(prerelease 1)
					if(build)
						set(error 1)
					else()
						if(patch)
							if(newpatch EQUAL 9999)
								math(EXPR patch "${patch} - 1")
								if(newbuild EQUAL 9999)
									set(build 9999)
								else()
									math(EXPR build "${newbuild} + 1000")
								endif()
							else()
								set(error 1)
							endif()
						else()
							if(minor)
								math(EXPR minor "${minor} - 1")
							else()
								if(major)
									math(EXPR major "${major} - 1")
								else()
									set(error 1)
								endif()
								set(minor 9999)
							endif()
							set(patch "${newpatch}")
							set(build "${newbuild}")
						endif()
					endif()
				endif()
				
				set(number "${major}")
				if(minor OR patch OR build)
					set(number "${number}.${minor}")
					if(patch OR build)
						set(number "${number}.${patch}")
						if(build)
							set(number "${number}.${build}")
						endif()
					endif()
				endif()
				
				_define_version_line_var(MAJOR major)
				_define_version_line_var(MINOR minor)
				_define_version_line_var(PATCH patch)
				_define_version_line_var(BUILD build)
				_define_version_line_var(PRERELEASE prerelease)
				_define_version_line_var(PRIVATE private)
				_define_version_line_var(NUMBER number)
				_define_version_line_var(ERROR error)
				
			endif()
			
		endif()
		
		math(EXPR i "${i} + 1")
	endforeach()
	
	set(${var}_COUNT ${i} PARENT_SCOPE)
	
	string(REGEX REPLACE "\n\n.*$" "" head "${contents}")
	_define_version_var(HEAD head)
	
	string(REGEX MATCH "\n\n.*" tail "${contents}")
	_define_version_var(TAIL tail)
	
	foreach(var IN LISTS variables)
		set(${var} "${${var}}" PARENT_SCOPE)
	endforeach()
	
endfunction()
