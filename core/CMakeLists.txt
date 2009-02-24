PROJECT (MOONSHINE_CORE C)

ADD_EXECUTABLE(moonshine 
	term.c
	async-queue-source.c
	signal.c
	lua.c
	main.c)

TARGET_LINK_LIBRARIES(moonshine
	${SLANG_LIBRARY}
	${GLIB_LIBRARIES}
	${LUA_LIBRARIES})

#ADD_EXECUTABLE(async-queue-test
#	ms-async-queue-source.c
#	async-queue-test.c)
#
#TARGET_LINK_LIBRARIES(async-queue-test
#	${GLIB_LIBRARIES})	

INCLUDE_DIRECTORIES ( ${MOONSHINE_CORE_SOURCE_DIR} )

INSTALL(TARGETS moonshine RUNTIME DESTINATION bin)
