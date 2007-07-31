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

IF (SLANG_FOUND)
	INCLUDE (cmake/SlangVersion.cmake)

	IF (SLANG_VERSION LESS 20000)
		MESSAGE(SEND_ERROR "moonshine requires slang-2")
	ELSE ()
		MESSAGE(STATUS "Good, slang is version 2")
	ENDIF ()
ENDIF()
