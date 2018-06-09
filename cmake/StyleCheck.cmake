
# Copyright (C) 2013-2018 Daniel Scharrer
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

find_package(PythonInterp)

set(STYLE_FILTER)

# Complains about any c-style cast -> too annoying.
set(STYLE_FILTER ${STYLE_FILTER},-readability/casting)

# Insists on including evrything in the .cpp file even if it is included in the header.
set(STYLE_FILTER ${STYLE_FILTER},-build/include_what_you_use)

# Too many false positives and not very helpful error messages.
set(STYLE_FILTER ${STYLE_FILTER},-build/include_order)

# No thanks.
set(STYLE_FILTER ${STYLE_FILTER},-readability/streams)

# Ugh!
set(STYLE_FILTER ${STYLE_FILTER},-whitespace/tab)

# Yes it is!
set(STYLE_FILTER ${STYLE_FILTER},-whitespace/blank_line)

# Suggessts excessive indentation.
set(STYLE_FILTER ${STYLE_FILTER},-whitespace/labels)

# Disallows brace on new line after long class memeber init list
set(STYLE_FILTER ${STYLE_FILTER},-whitespace/braces)

# Don't tell me how to name my variables.
set(STYLE_FILTER ${STYLE_FILTER},-runtime/arrays)

# Why?
set(STYLE_FILTER ${STYLE_FILTER},-whitespace/todo)
set(STYLE_FILTER ${STYLE_FILTER},-readability/todo)

# Annoyting to use with boost::program_options
set(STYLE_FILTER ${STYLE_FILTER},-whitespace/semicolon)

get_filename_component(STYLE_CHECK_DIR "${CMAKE_CURRENT_LIST_FILE}" PATH)
set(STYLE_CHECK_SCRIPT "${STYLE_CHECK_DIR}/cpplint.py")

# Add a target that runs cpplint.py
#
# Parameters:
# - TARGET_NAME the name of the target to add
# - SOURCES_LIST a complete list of source and include files to check
function(add_style_check_target TARGET_NAME SOURCES_LIST PROJECT)
	
	if(NOT PYTHONINTERP_FOUND)
		return()
	endif()
	
	list(SORT SOURCES_LIST)
	list(REMOVE_DUPLICATES SOURCES_LIST)
	
	add_custom_target(${TARGET_NAME}
		COMMAND "${CMAKE_COMMAND}" -E chdir
			"${PROJECT_SOURCE_DIR}"
			"${PYTHON_EXECUTABLE}"
			"${STYLE_CHECK_SCRIPT}"
			"--filter=${STYLE_FILTER}"
			"--project=${PROJECT}"
			${SOURCES_LIST}
		DEPENDS ${SOURCES_LIST} ${STYLE_CHECK_SCRIPT}
		COMMENT "Checking code style."
		VERBATIM
	)
	
endfunction(add_style_check_target)
