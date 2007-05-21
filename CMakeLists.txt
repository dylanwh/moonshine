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
	MESSAGE(FATAL_ERROR "spoon requires gcc to compile")
ELSE()
	MESSAGE(STATUS "Good, we have gcc")
ENDIF (NOT CMAKE_C_COMPILER MATCHES "gcc")

IF (NOT PERL_EXECUTABLE)
	MESSAGE(FATAL_ERROR "spoon requires perl")
ELSE()	
	MESSAGE(STATUS "Good, we have perl")
ENDIF (NOT PERL_EXECUTABLE)

# Debian renames lua to lua50 or lua5.1, etc.
# We execute findlua.pl to find the right pkg-config package.
EXEC_PROGRAM(${PERL_EXECUTABLE}
	ARGS ${CMAKE_CURRENT_SOURCE_DIR}/util/findlua.pl
	OUTPUT_VARIABLE LUA)

IF (NOT LUA)
	MESSAGE(FATAL_ERROR "spoon requires lua")
ELSE()
	MESSAGE(STATUS "Good, we have lua (${LUA})")
	IF (NOT ${LUA} STREQUAL "lua")
		MESSAGE(STATUS "  This smells like debian")
	ENDIF (NOT ${LUA} STREQUAL "lua")
ENDIF(NOT LUA)

# As far as I know, the *_INCDIR and *_LIBDIR variables are useless.
PKGCONFIG (${LUA}   LUA_INCDIR LUA_LIBDIR LUA_LDFLAGS LUA_CFLAGS)
PKGCONFIG (glib-2.0 GLIB_INCDIR GLIB_LIBDIR GLIB_LDFLAGS GLIB_CFLAGS)
PKGCONFIG (gnet-2.0 GNET_INCDIR GNET_LIBDIR GNET_LDFLAGS GNET_CFLAGS)

# If only slang used pkg-config...
# On gentoo, /usr/include/slang-2 is where slang v2 is kept.
# In addition, the library name is slang-2.
# On debian, it's just called "slang" and the header is in /usr/include.
IF (EXISTS "/usr/include/slang-2")
	MESSAGE(STATUS "This smells like gentoo")
	SET (SLANG_CFLAGS "-I/usr/include/slang-2")
	SET (SLANG_LDFLAGS "-lslang-2")
ELSE()
	SET (SLANG_CFLAGS "")
	SET (SLANG_LDFLAGS "-lslang")
ENDIF (EXISTS "/usr/include/slang-2")

# Now, after all that trouble, we need to make sure we're really using slang2.
# Isn't this fun?
ADD_DEFINITIONS("-std=gnu99 -Werror -Wall -Winline")
ADD_DEFINITIONS(${SLANG_CFLAGS})
ADD_DEFINITIONS(${GLIB_CFLAGS})
ADD_DEFINITIONS(${GNET_CFLAGS})
ADD_DEFINITIONS(${LUA_CFLAGS})

#SET(DIR "${CMAKE_CURRENT_SOURCE_DIR}/util")
#TRY_RUN(SLANG_RUN_RESULT SLANG_COMPILE_RESULT
#	${DIR} ${DIR}/slang-version.c
#	OUTPUT_VARIABLE SLANG_VERSION)


SET_TARGET_PROPERTIES(spoon PROPERTIES LINK_FLAGS 
	"${GLIB_LDFLAGS} ${GNET_LDFLAGS} ${LUA_LDFLAGS} ${SLANG_LDFLAGS}")


INSTALL(TARGETS spoon RUNTIME DESTINATION bin)

