PROJECT (MOONSHINE_SRC C)

ADD_EXECUTABLE(moonshine 
    term.c
    async-queue-source.c
    signal.c
    lua.c
    lua_var.c
    log.c
    main.c
    parseopt.c
    purple.c

    modidle.c
    modlog_core.c
    modnet_client.c

    modparseopt_core.c
    modui_buffer.c
    modui_entry.c
    modui_label.c
    modui_term.c

    modpurple_core.c
    modpurple_account.c
    modpurple_conversations.c
    modpurple_conversation.c
    modpurple_plugins.c
    modpurple_plugin.c
    modpurple_roomlist.c
    modpurple_room.c

    avl/avl.c
    modtree.c

    modtimer.c

    preload.h
)
SET_TARGET_PROPERTIES(moonshine PROPERTIES 
    RUNTIME_OUTPUT_DIRECTORY ${MOONSHINE_BINARY_DIR})

TARGET_LINK_LIBRARIES(moonshine
    ncursesw
    ${GLIB_LIBRARIES}
    ${GNET_LIBRARIES}
    ${LUA_LIBRARIES}
    ${PURPLE_LIBRARIES})


ADD_EXECUTABLE(test_lua_var
    test_lua_var.c
    lua.c
    lua_var.c
)

SET_TARGET_PROPERTIES(test_lua_var PROPERTIES 
    RUNTIME_OUTPUT_DIRECTORY ${MOONSHINE_BINARY_DIR})

TARGET_LINK_LIBRARIES(test_lua_var
    ${GLIB_LIBRARIES}
    ${GNET_LIBRARIES}
    ${LUA_LIBRARIES}
    ${PURPLE_LIBRARIES})

ADD_CUSTOM_COMMAND(
    OUTPUT            ${CMAKE_CURRENT_BINARY_DIR}/parseopt.c
    COMMAND           ${RE2C} parseopt.re2c.c > ${CMAKE_CURRENT_BINARY_DIR}/parseopt.c
    MAIN_DEPENDENCY   parseopt.re2c.c
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

ADD_CUSTOM_COMMAND(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/preload.h
    COMMAND ${LUA} preload.lua ${PRELOAD_MODULES} > ${CMAKE_CURRENT_BINARY_DIR}/preload.h
    MAIN_DEPENDENCY preload.lua
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

ADD_DEFINITIONS("-W -Wall")

INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_BINARY_DIR} ${MOONSHINE_SRC_SOURCE_DIR} )

INCLUDE("CheckTypeSize")
CHECK_TYPE_SIZE(wchar_t WCHAR_T_SIZE)

SET(CMAKE_REQUIRED_INCLUDES "${GLIB_INCLUDE_DIRS}")
SET(CMAKE_EXTRA_INCLUDE_FILES "glib.h")
CHECK_TYPE_SIZE(gunichar GUNICHAR_SIZE)

IF(${WCHAR_T_SIZE} EQUAL ${GUNICHAR_SIZE})
    MESSAGE(STATUS "yay, sizeof(wchar_t) == sizeof(gunichar) == ${WCHAR_T_SIZE}!")
ELSE()
    MESSAGE(FATAL_ERROR "moonshine assumes wchar_t fits into a gunichar, which isn't the case on your platform.")
ENDIF()

CONFIGURE_FILE(config.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/moonshine/config.h
    @ONLY IMMEDIATE)

INSTALL(TARGETS moonshine RUNTIME DESTINATION bin)
