cmake_minimum_required(VERSION 2.8)

# CMake script that reads a VERSION file and the current git history and the calls configure_file().
# This is used by version_file() in VersionString.cmake

if((NOT DEFINED INPUT) OR (NOT DEFINED OUTPUT) OR (NOT DEFINED VERSION_FILE) OR (NOT DEFINED GIT_DIR))
	message(SEND_ERROR "Invalid arguments.")
endif()

file(READ "${VERSION_FILE}" BASE_VERSION)
string(STRIP "${BASE_VERSION}" BASE_VERSION)

if(EXISTS "${GIT_DIR}")
	
	file(READ "${GIT_DIR}/HEAD" GIT_HEAD)
	string(STRIP "${GIT_HEAD}" GIT_HEAD)
	
	unset(GIT_COMMIT)
	
	if("${GIT_HEAD}" MATCHES "^ref\\:")
		
		# Remove the first for characters from GIT_HEAD to get GIT_REF.
		# We can't use a length of -1 for string(SUBSTRING) as cmake < 2.8.5 doesn't support it.
		string(LENGTH "${GIT_HEAD}" GIT_HEAD_LENGTH)
		math(EXPR GIT_REF_LENGTH "${GIT_HEAD_LENGTH} - 4")
		string(SUBSTRING "${GIT_HEAD}" 4 ${GIT_REF_LENGTH} GIT_REF)
		
		string(STRIP "${GIT_REF}" GIT_REF)
		
		file(READ "${GIT_DIR}/${GIT_REF}" GIT_HEAD)
		string(STRIP "${GIT_HEAD}" GIT_HEAD)
	endif()
	
	string(REGEX MATCH "[0-9A-Za-z]+" GIT_COMMIT "${GIT_HEAD}")
	
	# Create variables for all prefixes of the git comit ID.
	if(GIT_COMMIT)
		string(TOLOWER "${GIT_COMMIT}" GIT_COMMIT)
		string(LENGTH "${GIT_COMMIT}" GIT_COMMIT_LENGTH)
		foreach(i RANGE "${GIT_COMMIT_LENGTH}")
			string(SUBSTRING "${GIT_COMMIT}" 0 ${i} GIT_COMMIT_PREFIX_${i})
		endforeach()
	endif()
	
endif()

configure_file("${INPUT}" "${OUTPUT}" ESCAPE_QUOTES)
