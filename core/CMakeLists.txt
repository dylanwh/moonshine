PROJECT (MOONSHINE_CORE C)

ADD_EXECUTABLE(moonshine 
	ms-util.c
	ms-term.c
	ms-async-queue-source.c
	ms-signal.c
	ms-lua.c
	ms-main.c)

TARGET_LINK_LIBRARIES(moonshine
	${GLIB_LIBRARY}
	${SLANG_LIBRARY}
	${LUA_LIBRARY}
	${GTHREAD_LIBRARY})


