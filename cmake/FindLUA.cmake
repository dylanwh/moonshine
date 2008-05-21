# Copyright (c) 2003-2007 FlashCode <flashcode@flashtux.org>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#

# - Find LUA
# This module finds if liblua is installed and determines where 
# the include files and libraries are.
#
# This code sets the following variables:
#
#  LUA_INCLUDE_DIR = path to where <lua.h> can be found
#  LUA_LIBRARY = path to where liblua.so* (and liblualib.so* for lua <can be found (on non glibc based systems)
#
#  LUA_FOUND = is liblua usable on system?

MARK_AS_ADVANCED(LUA_INCLUDE_DIR LUA_LIBRARY LUA_LIBRARY)

IF (LUA_LIBRARY AND LUA_INCLUDE_DIR)
	SET(LUA_FOUND TRUE)
ELSE (LUA_LIBRARY AND LUA_INCLUDE_DIR)
	IF (LUA_FOUND)
   		# Already in cache, be silent
   		SET(LUA_FIND_QUIETLY TRUE)
	ENDIF(LUA_FOUND)

	FIND_PATH(
        LUA51_INCLUDE_DIR lua.h
        PATHS /usr/include /usr/local/include /usr/pkg/include
        PATH_SUFFIXES lua51 lua5.1 lua-5.1
		)

	FIND_LIBRARY(
        LUA51_LIBRARY NAMES lua51 lua5.1 lua-5.1 lua
        PATHS /lib /usr/lib /usr/local/lib /usr/pkg/lib
		)

	IF(LUA51_INCLUDE_DIR AND LUA51_LIBRARY)
		SET(LUA_FOUND TRUE)
	ENDIF(LUA51_INCLUDE_DIR AND LUA51_LIBRARY)

	IF (LUA_FOUND)
  		SET(LUA_INCLUDE_DIR "${LUA51_INCLUDE_DIR}" CACHE PATH "Lua include path")
  		SET(LUA_LIBRARY "${LUA51_LIBRARY}" CACHE STRING "Lua library")
  		SET(LUA_VERSION "5.1")
   		IF (NOT LUA_FIND_QUIETLY)
      		MESSAGE(STATUS "Found lua: ${LUA_LIBRARY}")
   		ENDIF (NOT LUA_FIND_QUIETLY)
	ELSE (LUA_FOUND)
   		IF (LUA_FIND_REQUIRED)
      		MESSAGE(FATAL_ERROR "Could not find lua")
   		ENDIF (LUA_FIND_REQUIRED)
	ENDIF (LUA_FOUND)
ENDIF (LUA_LIBRARY AND LUA_INCLUDE_DIR)
