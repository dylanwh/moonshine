PROJECT (spoon C)

# Includes

INCLUDE (CheckIncludeFile)
INCLUDE (FindPerl)
INCLUDE (UsePkgConfig)
INCLUDE (CheckCSourceCompiles)

# Options
ENABLE_TESTING()
SET(CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS 1)

# Executables.

ADD_EXECUTABLE (spoon signal.c keyboard.c screen.c term.c protocol.c main.c
	buffer.c)

# Discover compiler flags.

SET(SLANG_CFLAGS "")
SET(SLANG_LDFLAGS "-lslang")

## We do this to have a chance finding slang on gentoo.
IF (EXISTS "/usr/include/slang-2/slang.h")
	SET (SLANG_CFLAGS "-I/usr/include/slang-2")
ENDIF()

## Debian renames lua to lua50 or lua5.1, etc.
## We execute findlua.pl to find the right pkg-config package.
EXEC_PROGRAM(${PERL_EXECUTABLE}
	ARGS ${CMAKE_CURRENT_SOURCE_DIR}/util/findlua.pl
	OUTPUT_VARIABLE LUA)

## As far as I know, the *_INCDIR and *_LIBDIR variables are useless.
PKGCONFIG (${LUA}   LUA_INCDIR LUA_LIBDIR LUA_LDFLAGS LUA_CFLAGS)
PKGCONFIG (glib-2.0 GLIB_INCDIR GLIB_LIBDIR GLIB_LDFLAGS GLIB_CFLAGS)
PKGCONFIG (gnet-2.0 GNET_INCDIR GNET_LIBDIR GNET_LDFLAGS GNET_CFLAGS)

## Now we add these the the flags passed to the C compiler.
ADD_DEFINITIONS("-std=gnu99 -Werror -Wall -Winline")
ADD_DEFINITIONS(${SLANG_CFLAGS})
ADD_DEFINITIONS(${GLIB_CFLAGS})
ADD_DEFINITIONS(${GNET_CFLAGS})
ADD_DEFINITIONS(${LUA_CFLAGS})

## And add the link flags to spoon.
SET_TARGET_PROPERTIES(spoon PROPERTIES LINK_FLAGS 
	"${GLIB_LDFLAGS} ${GNET_LDFLAGS} ${LUA_LDFLAGS} ${SLANG_LDFLAGS}")

# Install rules.
INSTALL(TARGETS spoon RUNTIME DESTINATION bin)

#ADD_TEST(test2 false)

# Sanity checks.
IF (NOT CMAKE_C_COMPILER MATCHES "gcc")
	MESSAGE(SEND_ERROR "spoon requires gcc to compile")
ELSE()
	MESSAGE(STATUS "Good, we have gcc")
ENDIF()

IF (NOT PKGCONFIG_EXECUTABLE)
	MESSAGE(SEND_ERROR "spoon requires pkg-config")
ELSE()
	MESSAGE(STATUS "Good, we have pkg-config")
ENDIF()

IF (NOT PERL_EXECUTABLE)
	MESSAGE(SEND_ERROR "spoon requires perl")
ELSE()	
	MESSAGE(STATUS "Good, we have perl")
ENDIF()

IF (NOT LUA)
	MESSAGE(SEND_ERROR "spoon requires lua")
ELSE()
	MESSAGE(STATUS "Good, we have lua (${LUA})")
ENDIF()

INCLUDE (util/SlangVersion.cmake)
IF (SLANG_VERSION LESS 20000)
	MESSAGE(SEND_ERROR "spoon requires slang-2")
ELSE ()
	MESSAGE(STATUS "Good, we have slang-2")
ENDIF ()
