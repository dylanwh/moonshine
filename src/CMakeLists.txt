PROJECT (MOONSHINE_SRC C)

ADD_EXECUTABLE(moonshine 
	term.c
	async-queue-source.c
	signal.c
	lua.c
	log.c
	main.c
	parseopt.c
	
	modloop.c
	modidle.c
	modnet_client.c
	modparseopt_core.c
	modui_buffer.c
	modui_entry.c
	modui_statusbar.c
	modui_term.c

	avl/avl.c
	modtree.c

	modtimer.c
	modlog_core.c
)
SET_TARGET_PROPERTIES(moonshine PROPERTIES 
	RUNTIME_OUTPUT_DIRECTORY ${MOONSHINE_BINARY_DIR})

TARGET_LINK_LIBRARIES(moonshine
	${SLANG_LIBRARY}
	${GLIB_LIBRARIES}
	${GNET_LIBRARIES}
	${LUA_LIBRARIES})

ADD_CUSTOM_COMMAND(
	OUTPUT            ${CMAKE_CURRENT_BINARY_DIR}/parseopt.c
	COMMAND           ${RE2C} parseopt.re2c.c > ${CMAKE_CURRENT_BINARY_DIR}/parseopt.c
	MAIN_DEPENDENCY   parseopt.re2c.c
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

CONFIGURE_FILE(config.h.in
	${CMAKE_CURRENT_BINARY_DIR}/moonshine/config.h
	@ONLY IMMEDIATE)

ADD_DEFINITIONS("-W -Wall")

#ADD_EXECUTABLE(async-queue-test
#	ms-async-queue-source.c
#	async-queue-test.c)
#
#TARGET_LINK_LIBRARIES(async-queue-test
#	${GLIB_LIBRARIES})	

INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_BINARY_DIR} ${MOONSHINE_SRC_SOURCE_DIR} )

INSTALL(TARGETS moonshine RUNTIME DESTINATION bin)
