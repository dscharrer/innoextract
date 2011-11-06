
# the name of the target operating system
set(CMAKE_SYSTEM_NAME Windows)

# Look for mingw32 compilers
if(DEFINED MINGW32_ROOT)
	set(MinGW32_ROOT "${MINGW32_ROOT}" CACHE INTERNAL)
else()
	find_path(MinGW32_ROOT mingw
		PATH_SUFFIXES
		i686-pc-mingw32
		i586-pc-mingw32
		i486-pc-mingw32
		i386-pc-mingw32
		i686-mingw32
		i586-mingw32
		i486-mingw32
		i386-mingw32
		mingw32
		PATHS
		/usr
		/usr/local
	)
endif()
mark_as_advanced(MinGW32_ROOT)

find_program(MinGW32_GCC NAMES
	i686-pc-mingw32-gcc
	i586-pc-mingw32-gcc
	i486-pc-mingw32-gcc
	i386-pc-mingw32-gcc
	i686-mingw32-gcc
	i586-mingw32-gcc
	i486-mingw32-gcc
	i386-mingw32-gcc
)
mark_as_advanced(MinGW32_GCC)
find_program(MinGW32_GXX NAMES
	i686-pc-mingw32-g++
	i586-pc-mingw32-g++
	i486-pc-mingw32-g++
	i386-pc-mingw32-g++
	i686-mingw32-g++
	i586-mingw32-g++
	i486-mingw32-g++
	i386-mingw32-g++
)
mark_as_advanced(MinGW32_GXX)
find_program(MinGW32_RC NAMES
	i686-pc-mingw32-windres
	i586-pc-mingw32-windres
	i486-pc-mingw32-windres
	i386-pc-mingw32-windres
	i686-mingw32-windres
	i586-mingw32-windres
	i486-mingw32-windres
	i386-mingw32-windres
)
mark_as_advanced(MinGW32_RC)

if((NOT MinGW32_GCC) OR (NOT MinGW32_GXX) OR (NOT MinGW32_RC) OR (NOT MinGW32_ROOT))
	message(FATAL_ERROR "mingw32 not found (found gcc=\"${MinGW32_GCC}\", g++=\"${MinGW32_GXX}\" rc=\"${MinGW32_RC}\" root=\"${MinGW32_ROOT}\")")
endif()

# which compilers to use for C and C++
set(CMAKE_C_COMPILER "${MinGW32_GCC}")
set(CMAKE_CXX_COMPILER "${MinGW32_GXX}")
set(CMAKE_RC_COMPILER "${MinGW32_RC}")

# here is the target environment located
set(CMAKE_FIND_ROOT_PATH "${MinGW32_ROOT}")

# adjust the default behaviour of the find_xxx() commands:
# search headers and libraries in the target environment, search 
# programs in the host environment
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY FIRST)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE FIRST)
