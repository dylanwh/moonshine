# - Try to find GTHREAD
# Once done this will define
#
#  GTHREAD_FOUND - system has GTHREAD
#  GTHREAD_INCLUDE_DIR - the GTHREAD include directory
#  GTHREAD_LIBRARY - Link these to use GTHREAD
#  GTHREAD_DEFINITIONS - Compiler switches required for using GTHREAD
#
#  Copyright (c) 2006 Andreas Schneider <mail@cynapses.org>
#  Copyright (c) 2006 Philippe Bernery <philippe.bernery@gmail.com>
#
#  Redistribution and use is allowed according to the terms of the New
#  BSD license.
#  For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#
MARK_AS_ADVANCED(GTHREAD_INCLUDE_DIR GTHREAD_LIBRARY GTHREAD_DEFINITIONS)

IF (GTHREAD_LIBRARY AND GTHREAD_INCLUDE_DIR)
  	# in cache already
  	SET(GTHREAD_FOUND TRUE)
  	SET(GTHREAD_INCLUDE_DIRS ${GTHREAD_INCLUDE_DIR})
ELSE (GTHREAD_LIBRARY AND GTHREAD_INCLUDE_DIR)
	# use pkg-config to get the directories and then use these values
	# in the FIND_PATH() and FIND_LIBRARY() calls
	INCLUDE(UsePkgConfig)

 	## Glib
	PKGCONFIG(gthread-2.0 _GTHREADIncDir _GTHREADLinkDir _GTHREADLinkFlags _GTHREADCflags)


	FIND_PATH(GTHREAD_INCLUDE_DIR
    	NAMES glib/gthread.h
    	PATHS ${_GTHREADIncDir}
    	${GLIB_INCLUDE_DIR}
    	/opt/gnome/include/gthread-2.0
    	/usr/include/gthread-2.0
    	/usr/local/include/gthread-2.0
    	/opt/local/include/gthread-2.0
    	/opt/local/lib/gthread-2.0/include
    	/sw/include/gthread-2.0)

	FIND_LIBRARY(GTHREAD_LIBRARY
    	NAMES gthread-2.0
    	PATHS ${_GTHREADLinkDir}
    	${GLIB_LIBRARY_DIR}
    	/opt/gnome/lib
    	/usr/lib
    	/usr/local/lib
    	/opt/local/lib
    	/sw/lib)


	IF (GTHREAD_LIBRARY AND GTHREAD_INCLUDE_DIR)
		SET(GTHREAD_DEFINITIONS ${_GTHREADCflags} 
			CACHE STRING "compiler switches required for gthread")
		SET(GTHREAD_INCLUDE_DIRS ${GTHREAD_INCLUDE_DIR})
    	SET(GTHREAD_FOUND TRUE)
	ENDIF ()


	IF (GTHREAD_FOUND)
    	IF (NOT GTHREAD_FIND_QUIETLY)
      		MESSAGE(STATUS "Found gthread: ${GTHREAD_LIBRARY}")
    	ENDIF (NOT GTHREAD_FIND_QUIETLY)
	ELSE (GTHREAD_FOUND)
    	IF (GTHREAD_FIND_REQUIRED)
      		MESSAGE(FATAL_ERROR "Could not find gthread")
    	ENDIF (GTHREAD_FIND_REQUIRED)
	ENDIF (GTHREAD_FOUND)

ENDIF (GTHREAD_LIBRARY AND GTHREAD_INCLUDE_DIR)
