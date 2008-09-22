PROJECT (MOONSHINE_CORE C)

ADD_EXECUTABLE(moonshine 
	ms-util.c
	ms-term.c
	ms-async-queue-source.c
	ms-signal.c
	ms-lua.c
	ms-main.c)

ADD_EXECUTABLE(async-queue-test
	ms-async-queue-source.c
	async-queue-test.c)

TARGET_LINK_LIBRARIES(moonshine
	${SLANG_LIBRARY}
	${GLIB_LIBRARIES}
	${LUA_LIBRARIES})

TARGET_LINK_LIBRARIES(async-queue-test
	${GLIB_LIBRARIES})	

INCLUDE_DIRECTORIES ( ${MOONSHINE_CORE_SOURCE_DIR} )

INSTALL(TARGETS moonshine RUNTIME DESTINATION bin)
