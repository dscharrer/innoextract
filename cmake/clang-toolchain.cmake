
# Look for wine compilers
find_program(Clang clang)
mark_as_advanced(Clang)
find_program(ClangXX clang++)
mark_as_advanced(ClangXX)

if((NOT Clang) OR (NOT ClangXX))
	message(FATAL_ERROR "clang not found (found: c compiler \"${Clang}\", c++ compiler \"${ClangXX}\")")
endif()

# which compilers to use for C and C++
set(CMAKE_C_COMPILER "${Clang}")
set(CMAKE_CXX_COMPILER "${ClangXX}")
