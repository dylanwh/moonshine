PROJECT (MOONSHINE_CORE C)

ADD_EXECUTABLE(moonshine 
	ms-util.c
	ms-term.c
	ms-async-queue-source.c
	ms-signal.c
	ms-lua.c
	ms-main.c)

TARGET_LINK_LIBRARIES(moonshine
	${SLANG_LIBRARY}
	${GLIB_LIBRARIES}
	${LUA_LIBRARIES})

#INSTALL(TARGETS moonshine RUNTIME DESTINATION bin)
