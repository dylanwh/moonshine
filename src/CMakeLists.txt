PROJECT (MOONSHINE_SRC C)

ADD_EXECUTABLE(moonshine 
	term.c
	async-queue-source.c
	signal.c
	lua.c
	main.c
	parseopt.c
	
	modloop.c
	modnet_client.c
	modparseopt_core.c
	modui_buffer.c
	modui_entry.c
	modui_statusbar.c
	modui_term.c

	avl/avl.c
	modtree.c
)

TARGET_LINK_LIBRARIES(moonshine
	${SLANG_LIBRARY}
	${GLIB_LIBRARIES}
	${GNET_LIBRARIES}
	${LUA_LIBRARIES})

ADD_CUSTOM_COMMAND(
	OUTPUT            ${MOONSHINE_SRC_BINARY_DIR}/parseopt.c
	COMMAND           ${RE2C} parseopt.re2c.c > ${MOONSHINE_SRC_BINARY_DIR}/parseopt.c
	MAIN_DEPENDENCY   parseopt.re2c.c
	WORKING_DIRECTORY ${MOONSHINE_SRC_SOURCE_DIR})

#ADD_EXECUTABLE(async-queue-test
#	ms-async-queue-source.c
#	async-queue-test.c)
#
#TARGET_LINK_LIBRARIES(async-queue-test
#	${GLIB_LIBRARIES})	

INCLUDE_DIRECTORIES( ${MOONSHINE_SRC_BINARY_DIR} ${MOONSHINE_SRC_SOURCE_DIR} )

INSTALL(TARGETS moonshine RUNTIME DESTINATION bin)
