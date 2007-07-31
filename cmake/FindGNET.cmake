# - Try to find GNET
# Once done this will define
#
#  GNET_FOUND - system has GNET
#  GNET_INCLUDE_DIR - the GNET include directory
#  GNET_LIBRARY - Link these to use GNET
#  GNET_DEFINITIONS - Compiler switches required for using GNET
#
#  Copyright (c) 2006 Andreas Schneider <mail@cynapses.org>
#  Copyright (c) 2006 Philippe Bernery <philippe.bernery@gmail.com>
#
#  Redistribution and use is allowed according to the terms of the New
#  BSD license.
#  For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#

IF (GNET_LIBRARY AND GNET_INCLUDE_DIR)
  	# in cache already
  	SET(GNET_FOUND TRUE)
    SET(GNET_INCLUDE_DIRS ${GNET_INCLUDE_DIR} ${GNETCONFIG_INCLUDE_DIR})
ELSE (GNET_LIBRARY AND GNET_INCLUDE_DIR)
  	# use pkg-config to get the directories and then use these values
  	# in the FIND_PATH() and FIND_LIBRARY() calls
  	INCLUDE(UsePkgConfig)

  	## Gnet
  	PKGCONFIG(gnet-2.0 _GNETIncDir _GNETLinkDir _GNETLinkFlags _GNETCflags)

	FIND_PATH(GNETCONFIG_INCLUDE_DIR
      	NAMES gnetconfig.h
      	PATHS
      	${_GNETIncDir}
      	${_GNETIncDir}include/gnet-2.0
      	${_GNETLinkDir}/gnet-2.0/include
      	/opt/gnome/include/gnet-2.0
      	/usr/include/gnet-2.0
      	/usr/local/include/gnet-2.0
      	/opt/local/include/gnet-2.0
      	/sw/include/gnet-2.0)

  	SET(GNET_DEFINITIONS "${_GNETCflags}" CACHE STRING "cflags required for gnet")
	
  	FIND_PATH(GNET_INCLUDE_DIR
      	NAMES gnet.h
      	PATHS ${_GNETIncDir}
      	${_GNETIncDir}
      	${_GNETIncDir}include/gnet-2.0
      	${_GNETLinkDir}/gnet-2.0/include
      	/opt/gnome/include/gnet-2.0
      	/usr/include/gnet-2.0
      	/usr/local/include/gnet-2.0
      	/opt/local/include/gnet-2.0
      	/sw/include/gnet-2.0)

  	FIND_LIBRARY(GNET_LIBRARY
      	NAMES gnet-2.0
      	PATHS ${_GNETLinkDir}
      	/opt/gnome/lib
      	/usr/lib
      	/usr/local/lib
      	/opt/local/lib
      	/sw/lib)

    IF (GNET_LIBRARY AND GNET_INCLUDE_DIR)
    	SET(GNET_INCLUDE_DIRS ${GNET_INCLUDE_DIR} ${GNETCONFIG_INCLUDE_DIR})
    	SET(GNET_FOUND TRUE)
    ENDIF (GNET_LIBRARY AND GNET_INCLUDE_DIR)

  	IF (GNET_FOUND)
    	IF (NOT GNET_FIND_QUIETLY)
      		MESSAGE(STATUS "Found GNET: ${GNET_LIBRARY}")
    	ENDIF (NOT GNET_FIND_QUIETLY)
  	ELSE (GNET_FOUND)
    	IF (GNET_FIND_REQUIRED)
      		MESSAGE(FATAL_ERROR "Could not find GNET")
    	ENDIF (GNET_FIND_REQUIRED)
  	ENDIF (GNET_FOUND)

  	MARK_AS_ADVANCED(GNET_INCLUDE_DIR GNETCONFIG_INCLUDE_DIR GNET_LIBRARY)
ENDIF (GNET_LIBRARY AND GNET_INCLUDE_DIR)
