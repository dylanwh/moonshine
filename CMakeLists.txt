PROJECT (MOONSHINE C)
INCLUDE(CheckFunctionExists)

SET(SRC "${MOONSHINE_SOURCE_DIR}")
SET(BIN "${MOONSHINE_BINARY_DIR}") 
SET(CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS 1)
SET(CMAKE_MODULE_PATH "${SRC}/cmake")

ADD_EXECUTABLE(moonshine term.c moon.c 
	buffer.c entry.c topic.c
	linereader.c
	util.c main.c handle.c net.c signal.c)

OPTION(EMBED_LUA "Toggles the embeding of lua packages in the executable" OFF)

IF (EMBED_LUA)
	MESSAGE(STATUS "Will embed lua packages")
ELSE()
	MESSAGE(STATUS "Will not embed lua packages")
ENDIF()

FIND_PACKAGE(LUA REQUIRED)
FIND_PACKAGE(SLANG REQUIRED)
FIND_PACKAGE(GLIB REQUIRED)
FIND_PACKAGE(GTHREAD REQUIRED)

INCLUDE_DIRECTORIES(BEFORE ${SLANG_INCLUDE_DIR} ${LUA_INCLUDE_DIR} ${BIN}
	${GLIB_INCLUDE_DIRS} ${GTHREAD_INCLUDE_DIRS} ${SRC})	

TARGET_LINK_LIBRARIES(moonshine ${GLIB_LIBRARY} ${SLANG_LIBRARY}
	${LUA_LIBRARY} ${GTHREAD_LIBRARY})

## Now we add these the the flags passed to the C compiler.
ADD_DEFINITIONS("-std=gnu99 -Wall -Winline -I${BIN}")
ADD_DEFINITIONS("${GLIB_DEFINITIONS} ${GTHREAD_DEFINITIONS}")

CONFIGURE_FILE(${SRC}/config.h.in
	${BIN}/config.h
	@ONLY IMMEDIATE)

IF (EMBED_LUA)
	FILE(GLOB PACKAGES ${SRC}/lua/*.lua)
	ADD_CUSTOM_COMMAND(OUTPUT ${BIN}/packages.h
		COMMAND perl ${SRC}/cmake/embed-packages.pl 
		${PACKAGES} > ${BIN}/packages.h
		MAIN_DEPENDENCY ${SRC}/cmake/embed-packages.pl DEPENDS ${PACKAGES}
		WORKING_DIRECTORY ${SRC})

	SET_SOURCE_FILES_PROPERTIES(moon.c PROPERTIES OBJECT_DEPENDS "${BIN}/packages.h")
ENDIF()

# Install rules.
INSTALL(TARGETS moonshine RUNTIME DESTINATION bin)

INCLUDE(CheckRequired)
