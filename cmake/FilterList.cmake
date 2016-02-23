
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

# Filter a list with conditional ites
#
# Supported syntax:
#
#  item
#
#  item if CONDITION_VARIABLE
#
#  CONDITION_VARIABLE {
#    item1
#    item2
#  }
#
# Conditions cannot be nested.
#
# The list specified by LIST_NAM Ewill be modifed in place.
# An optional second parameter can be given to specify the name of a list that
# should receive all items, even those whose conditions evaluated to false,
# but not syntactic elements such as 'if', '{', '}' and condition variables.
#
function(filter_list LIST_NAME)
	
	set(TOKEN_IF "if")
	set(TOKEN_GROUP_BEGIN "{")
	set(TOKEN_GROUP_END "}")
	
	set(filtered)
	set(all)
	
	# the item from the previous iteration
	set(last_item)
	
	# current syntax state:
	# 0 - start
	# 1 - after 'if', expected condition variable
	# 2 - inside block (true)
	# 3 - inside block (false)
	set(mode 0)
	
	foreach(item IN LISTS ${LIST_NAME})
		
		if(mode EQUAL 1)
			
			# Handle condition variables
			set(condition ${${item}})
			if(condition)
				list(APPEND filtered ${last_item})
			endif()
			set(mode 0)
			set(last_item)
			
		elseif(item STREQUAL TOKEN_IF)
			
			if(NOT mode EQUAL 0)
				message(FATAL_ERROR "bad filter_list syntax: IF inside { } block is forbidden")
			endif()
			
			# Handle condition start
			if("${last_item}" STREQUAL "")
				message(FATAL_ERROR "bad filter_list syntax: IF without preceding item")
			endif()
			set(mode 1)
			
		elseif(item STREQUAL TOKEN_GROUP_BEGIN)
			
			if(NOT mode EQUAL 0)
				message(FATAL_ERROR "bad filter_list syntax: cannot nest { } blocks")
			endif()
			
			if(last_item STREQUAL "")
				message(FATAL_ERROR "bad filter_list syntax: { without preceding item")
			endif()
			
			set(condition ${${last_item}})
			if(condition)
				set(mode 2)
			else()
				set(mode 3)
			endif()
			set(last_item)
			
		else()
			
			# Handle unconditional items
			if(NOT last_item STREQUAL "" AND NOT mode EQUAL 3)
				list(APPEND filtered ${last_item})
			endif()
			
			if(item STREQUAL TOKEN_GROUP_END)
				
				if(mode EQUAL 0)
					message(FATAL_ERROR "bad filter_list syntax: } without open block")
				endif()
				
				set(mode 0)
				set(last_item)
				
			else()
				
				list(APPEND all ${item})
				set(last_item ${item})
				
			endif()
			
		endif()
		
	endforeach()
	
	if(mode EQUAL 1)
		message(FATAL_ERROR "bad filter_list syntax: unexpected end, expected condition")
	elseif(mode EQUAL 2 OR mode EQUAL 3)
		message(FATAL_ERROR "bad filter_list syntax: unexpected end, expected }")
	endif()
	
	list(SORT filtered)
	list(REMOVE_DUPLICATES filtered)
	set(${LIST_NAME} ${filtered} PARENT_SCOPE)
	
	if(ARGC GREATER 1)
		list(SORT all)
		list(REMOVE_DUPLICATES all)
		set(${ARGV1} ${all} PARENT_SCOPE)
	endif()
	
endfunction()
