PROJECT (moonshine C)

ADD_EXECUTABLE (moonshine moon.c signal.c keyboard.c screen.c term.c protocol.c main.c 
	buffer.c entry.c)

# Includes
INCLUDE (CheckIncludeFile)
INCLUDE (FindDoxygen)
INCLUDE (UsePkgConfig)
INCLUDE (CheckCSourceCompiles)

# Options
# ENABLE_TESTING()
SET(CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS 1)


## As far as I know, the *_INCDIR and *_LIBDIR variables are useless.
PKGCONFIG (glib-2.0 GLIB_INCDIR GLIB_LIBDIR GLIB_LDFLAGS GLIB_CFLAGS)
PKGCONFIG (gnet-2.0 GNET_INCDIR GNET_LIBDIR GNET_LDFLAGS GNET_CFLAGS)
FOREACH(package bob lua5.1 lua50 lua)
	IF (NOT LUA_LDFLAGS)
		PKGCONFIG (${package} LUA_INCDIR LUA_LIBDIR LUA_LDFLAGS LUA_CFLAGS)
		SET(LUA "${package}")
	ENDIF ()
ENDFOREACH ()

SET(SLANG_CFLAGS "")
SET(SLANG_LDFLAGS "-lslang")
IF (EXISTS "/usr/include/slang-2/slang.h")
	MESSAGE(STATUS "Hmm, gentoo hides slang.h in its underpants...")
	SET (SLANG_CFLAGS "-I/usr/include/slang-2")
	SET (SLANG_LDFLAGS "-lslang-2")
ENDIF()

## Now we add these the the flags passed to the C compiler.
ADD_DEFINITIONS("-std=gnu99 -Werror -Wall -Winline")
ADD_DEFINITIONS(${SLANG_CFLAGS})
ADD_DEFINITIONS(${GLIB_CFLAGS})
ADD_DEFINITIONS(${GNET_CFLAGS})
ADD_DEFINITIONS(${LUA_CFLAGS})

# And add the link flags to moonshine.
SET_TARGET_PROPERTIES(moonshine PROPERTIES LINK_FLAGS 
	"${GLIB_LDFLAGS} ${GNET_LDFLAGS} ${LUA_LDFLAGS} ${SLANG_LDFLAGS}")

CONFIGURE_FILE(${moonshine_SOURCE_DIR}/docs/Doxyfile.in
	${moonshine_BINARY_DIR}/docs/Doxyfile
	@ONLY IMMEDIATE)

ADD_CUSTOM_TARGET(api-docs ALL ${DOXYGEN} docs/Doxyfile)

# Install rules.
INSTALL(TARGETS moonshine RUNTIME DESTINATION bin)

INCLUDE(util/CheckRequired.cmake)
