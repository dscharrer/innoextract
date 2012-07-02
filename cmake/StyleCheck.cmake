
find_package(PythonInterp)

unset(STYLE_FILTER)

# Complains about any c-style cast -> too annoying.
set(STYLE_FILTER ${STYLE_FILTER},-readability/casting)

# Insists on including evrything in the .cpp file even if it is included in the header.
# This behaviour conflicts with orther tools.
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

# Don't tell me how to name my variables.
set(STYLE_FILTER ${STYLE_FILTER},-runtime/arrays)

# Why?
set(STYLE_FILTER ${STYLE_FILTER},-whitespace/todo)
set(STYLE_FILTER ${STYLE_FILTER},-readability/todo)

# Annoyting to use with boost::program_options
set(STYLE_FILTER ${STYLE_FILTER},-whitespace/semicolon)

# Add a target that runs cpplint.py
#
# Parameters:
# - TARGET_NAME the name of the target to add
# - SOURCES_LIST a complete list of source files to check
# - INCLUDES_LIST a complete list of include files to check
function(add_style_check_target TARGET_NAME SOURCES_LIST INCLUDES_LIST)
	
	if(NOT PYTHONINTERP_FOUND)
		return()
	endif()
	
	add_custom_target(${TARGET_NAME}
		COMMAND cmake -E chdir
			"${CMAKE_SOURCE_DIR}"
			"${PYTHON_EXECUTABLE}"
			"${CMAKE_MODULE_PATH}/cpplint.py"
			"--filter=${STYLE_FILTER}"
			${SOURCES_LIST} ${INCLUDES_LIST}
		DEPENDS ${SOURCES_LIST} ${INCLUDES_LIST} VERBATIM
	)
	
endfunction(add_style_check_target)
