INCLUDE (CheckIncludeFile)
#INCLUDE (CheckSymbolExists)
INCLUDE (FindPerl)
INCLUDE (UsePkgConfig)
INCLUDE (CheckCSourceCompiles)

PROJECT (spoon C)

ADD_EXECUTABLE (spoon signal.c keyboard.c screen.c term.c protocol.c main.c
	buffer.c)

# Sanity checks.
IF (NOT CMAKE_C_COMPILER MATCHES "gcc")
	MESSAGE(SEND_ERROR "spoon requires gcc to compile")
ELSE()
	MESSAGE(STATUS "Good, we have gcc")
ENDIF (NOT CMAKE_C_COMPILER MATCHES "gcc")

IF (NOT PERL_EXECUTABLE)
	MESSAGE(SEND_ERROR "spoon requires perl")
ELSE()	
	MESSAGE(STATUS "Good, we have perl")
ENDIF (NOT PERL_EXECUTABLE)

# Debian renames lua to lua50 or lua5.1, etc.
# We execute findlua.pl to find the right pkg-config package.
EXEC_PROGRAM(${PERL_EXECUTABLE}
	ARGS ${CMAKE_CURRENT_SOURCE_DIR}/util/findlua.pl
	OUTPUT_VARIABLE LUA)

IF (NOT LUA)
	MESSAGE(SEND_ERROR "spoon requires lua")
ELSE()
	MESSAGE(STATUS "Good, we have lua (${LUA})")
	IF (NOT ${LUA} STREQUAL "lua")
		MESSAGE(STATUS "  This smells like debian")
	ENDIF (NOT ${LUA} STREQUAL "lua")
ENDIF(NOT LUA)

SET(SLANG_CFLAGS "")
SET(SLANG_LDFLAGS "-lslang")

INCLUDE (util/SlangVersion.cmake)
IF (SLANG_VERSION LESS 20006)
	MESSAGE(STATUS "Hmm, you slang seems old. Let's see if you have a newer one.")
	IF (EXISTS "/usr/include/slang-2/slang.h")
		MESSAGE(STATUS "Found /usr/include/slang-2/slang.h. Do I smell gentoo?")
		SET(SLANG_CFLAGS "-I/usr/include/slang-2")
		MESSAGE(STATUS "I also assume I should use -lslang-2 instead of -lslang.")
		SET(SLANG_LDFLAGS "-lslang-2")
		MESSAGE(STATUS "Now to check the slang version again.")
		SET(CMAKE_REQUIRED_FLAGS "${SLANG_CFLAGS}")
		INCLUDE (util/SlangVersion.cmake)
		IF (SLANG_VERSION LESS 20006)
			MESSAGE(SEND_ERROR "Fudge bunnies. I can't find a decent version of slang. I give up.")
		ELSE ()
			MESSAGE(STATUS "Good, I managed to find your hidden slang-2 header file, evil gentoo user!")
		ENDIF (SLANG_VERSION LESS 20006)
	ENDIF (EXISTS "/usr/include/slang-2/slang.h")
ELSE ()
	MESSAGE(STATUS "Good, we have slang-2")
ENDIF (SLANG_VERSION LESS 20006)



# As far as I know, the *_INCDIR and *_LIBDIR variables are useless.
PKGCONFIG (${LUA}   LUA_INCDIR LUA_LIBDIR LUA_LDFLAGS LUA_CFLAGS)
PKGCONFIG (glib-2.0 GLIB_INCDIR GLIB_LIBDIR GLIB_LDFLAGS GLIB_CFLAGS)
PKGCONFIG (gnet-2.0 GNET_INCDIR GNET_LIBDIR GNET_LDFLAGS GNET_CFLAGS)

ADD_DEFINITIONS("-std=gnu99 -Werror -Wall -Winline")
ADD_DEFINITIONS(${SLANG_CFLAGS})
ADD_DEFINITIONS(${GLIB_CFLAGS})
ADD_DEFINITIONS(${GNET_CFLAGS})
ADD_DEFINITIONS(${LUA_CFLAGS})

SET_TARGET_PROPERTIES(spoon PROPERTIES LINK_FLAGS 
	"${GLIB_LDFLAGS} ${GNET_LDFLAGS} ${LUA_LDFLAGS} ${SLANG_LDFLAGS}")


INSTALL(TARGETS spoon RUNTIME DESTINATION bin)

