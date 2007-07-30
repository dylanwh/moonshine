PROJECT (MOONSHINE C)

ADD_EXECUTABLE (moonshine term.c packages.h moon.c buffer.c entry.c
	modEntry.c modBuffer.c modapp.c util.c main.c)

# Includes
INCLUDE (CheckIncludeFile)
INCLUDE (FindDoxygen)
INCLUDE (UsePkgConfig)
INCLUDE (CheckCSourceCompiles)

SET(SRC "${MOONSHINE_SOURCE_DIR}")
SET(BIN "${MOONSHINE_BINARY_DIR}") 

# Options
# ENABLE_TESTING()
SET(CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS 1)

## As far as I know, the *_INCDIR and *_LIBDIR variables are useless.
PKGCONFIG (glib-2.0 GLIB_INCDIR GLIB_LIBDIR GLIB_LDFLAGS GLIB_CFLAGS)
PKGCONFIG (gnet-2.0 GNET_INCDIR GNET_LIBDIR GNET_LDFLAGS GNET_CFLAGS)
FOREACH(package lua5.1 lua50 lua)
	IF (NOT LUA_LDFLAGS)
		PKGCONFIG (${package} LUA_INCDIR LUA_LIBDIR LUA_LDFLAGS LUA_CFLAGS)
		IF (LUA_LDFLAGS)
			SET(LUA "${package}")
		ENDIF ()
	ENDIF ()
ENDFOREACH ()

SET(SLANG_CFLAGS "")
SET(SLANG_LDFLAGS "-lslang")

IF (EXISTS "/usr/include/slang-2/slang.h")
	MESSAGE(STATUS "Hmm, gentoo hides slang.h in its underpants...")
	SET (SLANG_CFLAGS "-I/usr/include/slang-2")
	SET (SLANG_LDFLAGS "-lslang-2")
ENDIF()

IF (EXISTS "/opt/local/include/slang.h")
	MESSAGE(STATUS "I small darwin or macosx ports.")
	SET (SLANG_CFLAGS "-I/opt/local/include")
	SET (SLANG_LDFLAGS "-L/opt/local/lib -lslang")
ENDIF()

## Now we add these the the flags passed to the C compiler.
ADD_DEFINITIONS("-std=gnu99 -Werror -Wall -Winline -I${BIN}")
ADD_DEFINITIONS(${SLANG_CFLAGS})
ADD_DEFINITIONS(${GLIB_CFLAGS})
ADD_DEFINITIONS(${GNET_CFLAGS})
ADD_DEFINITIONS(${LUA_CFLAGS})


# And add the link flags to moonshine.
SET_TARGET_PROPERTIES(moonshine PROPERTIES LINK_FLAGS 
	"${GLIB_LDFLAGS} ${GNET_LDFLAGS} ${LUA_LDFLAGS} ${SLANG_LDFLAGS}")

CONFIGURE_FILE(${SRC}/docs/Doxyfile.in
	${BIN}/docs/Doxyfile
	@ONLY IMMEDIATE)

# ADD_CUSTOM_TARGET(api-docs ALL ${DOXYGEN} docs/Doxyfile)

FILE(GLOB PACKAGES ${SRC}/lua/*.lua)
ADD_CUSTOM_COMMAND(OUTPUT ${BIN}/packages.h
	COMMAND perl ${SRC}/cmake/embed-packages.pl 
	${PACKAGES} > ${BIN}/packages.h
	MAIN_DEPENDENCY ${SRC}/cmake/embed-packages.pl DEPENDS ${PACKAGES}
	WORKING_DIRECTORY ${SRC})

SET_SOURCE_FILES_PROPERTIES(moon.c PROPERTIES OBJECT_DEPENDS
	"${BIN}/packages.h")


# Install rules.
INSTALL(TARGETS moonshine RUNTIME DESTINATION bin)

INCLUDE(cmake/CheckRequired.cmake)


