# - Try to find GLIB
# Once done this will define
#
#  GLIB_FOUND - system has GLIB
#  GLIB_INCLUDE_DIR - the GLIB include directory
#  GLIB_LIBRARY - Link these to use GLIB
#  GLIB_DEFINITIONS - Compiler switches required for using GLIB
#
#  Copyright (c) 2006 Andreas Schneider <mail@cynapses.org>
#  Copyright (c) 2006 Philippe Bernery <philippe.bernery@gmail.com>
#
#  Redistribution and use is allowed according to the terms of the New
#  BSD license.
#  For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#

IF (GLIB_LIBRARY AND GLIB_INCLUDE_DIR)
  	# in cache already
  	SET(GLIB_FOUND TRUE)
  	SET(GLIB_INCLUDE_DIRS ${GLIB_INCLUDE_DIR} ${GLIBCONFIG_INCLUDE_DIR})
ELSE (GLIB_LIBRARY AND GLIB_INCLUDE_DIR)
	# use pkg-config to get the directories and then use these values
	# in the FIND_PATH() and FIND_LIBRARY() calls
	INCLUDE(UsePkgConfig)

 	## Glib
	PKGCONFIG(glib-2.0 _GLIBIncDir _GLIBLinkDir _GLIBLinkFlags _GLIBCflags)

	FIND_PATH(GLIBCONFIG_INCLUDE_DIR
 	    NAMES glibconfig.h
 	    PATHS ${_GLIB2IncDir}
 	    /opt/gnome/lib64/glib-2.0/include
 	    /opt/gnome/lib/glib-2.0/include
 	    /opt/lib/glib-2.0/include
 	    /usr/lib64/glib-2.0/include
 	    /usr/lib/glib-2.0/include
 	    /opt/local/lib/glib-2.0/include
 	    /sw/lib/glib-2.0/include)



	FIND_PATH(GLIB_INCLUDE_DIR
    	NAMES glib.h
    	PATHS ${_GLIBIncDir}
    	/opt/gnome/include/glib-2.0
    	/usr/include/glib-2.0
    	/usr/local/include/glib-2.0
    	/opt/local/include/glib-2.0
    	/opt/local/lib/glib-2.0/include
    	/sw/include/glib-2.0)

	FIND_LIBRARY(GLIB_LIBRARY
    	NAMES glib-2.0
    	PATHS ${_GLIBLinkDir}
    	/opt/gnome/lib
    	/usr/lib
    	/usr/local/lib
    	/opt/local/lib
    	/sw/lib)

	IF (GLIB_LIBRARY AND GLIB_INCLUDE_DIR AND GLIBCONFIG_INCLUDE_DIR)
		SET(GLIB_DEFINITIONS ${_GLIBCflags} 
			CACHE STRING "compiler switches required for glib")
		SET(GLIB_INCLUDE_DIRS ${GLIB_INCLUDE_DIR} ${GLIBCONFIG_INCLUDE_DIR})
    	SET(GLIB_FOUND TRUE)
	ENDIF ()

	IF (GLIB_FOUND)
    	IF (NOT GLIB_FIND_QUIETLY)
      		MESSAGE(STATUS "Found glib: ${GLIB_LIBRARY}")
    	ENDIF (NOT GLIB_FIND_QUIETLY)
	ELSE (GLIB_FOUND)
    	IF (GLIB_FIND_REQUIRED)
      		MESSAGE(FATAL_ERROR "Could not find glib")
    	ENDIF (GLIB_FIND_REQUIRED)
	ENDIF (GLIB_FOUND)

	MARK_AS_ADVANCED(GLIB_INCLUDE_DIR GLIBCONFIG_INCLUDE_DIR GLIB_LIBRARY)
ENDIF (GLIB_LIBRARY AND GLIB_INCLUDE_DIR)
