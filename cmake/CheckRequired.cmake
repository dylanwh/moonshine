# Sanity checks.
IF (NOT CMAKE_C_COMPILER MATCHES "gcc")
	MESSAGE(SEND_ERROR "moonshine requires gcc to compile")
ELSE()
	MESSAGE(STATUS "Good, we have gcc")
ENDIF()

IF (NOT PKGCONFIG_EXECUTABLE)
	MESSAGE(SEND_ERROR "moonshine requires pkg-config")
ELSE()
	MESSAGE(STATUS "Good, we have pkg-config")
ENDIF()

IF (NOT GLIB_LDFLAGS)
	MESSAGE(SEND_ERROR "moonshine requires glib-2.0")
ELSE()
	MESSAGE(STATUS "Good, we have glib")
ENDIF()

IF (NOT GNET_LDFLAGS)
	MESSAGE(SEND_ERROR "moonshine requires gnet-2.0")
ELSE()
	MESSAGE(STATUS "Good, we have gnet")
ENDIF()

IF (NOT LUA)
	MESSAGE(SEND_ERROR "moonshine requires lua")
ELSE()
	MESSAGE(STATUS "Good, we have lua (${LUA})")
ENDIF()

INCLUDE (cmake/SlangVersion.cmake)
IF (SLANG_VERSION LESS 20000)
	MESSAGE(SEND_ERROR "moonshine requires slang-2")
ELSE ()
	MESSAGE(STATUS "Good, we have slang-2")
ENDIF ()


