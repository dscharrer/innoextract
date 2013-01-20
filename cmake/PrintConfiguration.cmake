
# Copyright (C) 2013 Daniel Scharrer
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

function(print_configuration TITLE)
	
	set(str "")
	
	set(print_first 0)
	
	set(mode 0)
	
	foreach(arg IN LISTS ARGN)
		
		if(arg STREQUAL "FIRST")
			set(print_first 1)
		else()
			
			if(mode EQUAL 0)
				
				if(${arg})
					set(mode 1)
				else()
					set(mode 2)
				endif()
				
			else()
				
				if(mode EQUAL 1 AND NOT arg STREQUAL "")
					
					if(str STREQUAL "")
						set(str "${arg}")
					else()
						set(str "${str}, ${arg}")
					endif()
					
					if(print_first)
						break()
					endif()
					
				endif()
				
				set(mode 0)
				
			endif()
			
		endif()
		
	endforeach()
	
	if(str STREQUAL "")
		set(str "(none)")
	endif()
	
	message(" - ${TITLE}: ${str}")
	
endfunction()
