MARK_AS_ADVANCED(SLANG_INCLUDE_DIR SLANG_LIBRARY)

IF (SLANG_LIBRARY AND SLANG_INCLUDE_DIR)
	SET(SLANG_FOUND TRUE)
ELSE (SLANG_LIBRARY AND SLANG_INCLUDE_DIR)
	FIND_PATH(SLANG_INCLUDE_DIR slang.h
		/usr/include/slang-2)

	FIND_LIBRARY(SLANG_LIBRARY 
		NAMES slang-2 slang 
		PATH /usr/lib /usr/local/lib) 

	IF (SLANG_INCLUDE_DIR AND SLANG_LIBRARY)
   		SET(SLANG_FOUND TRUE)
	ENDIF (SLANG_INCLUDE_DIR AND SLANG_LIBRARY)

	IF (SLANG_FOUND)
   		IF (NOT SLANG_FIND_QUIETLY)
      		MESSAGE(STATUS "Found slang: ${SLANG_LIBRARY}")
   		ENDIF (NOT SLANG_FIND_QUIETLY)
	ELSE (SLANG_FOUND)
   		IF (SLANG_FIND_REQUIRED)
      		MESSAGE(FATAL_ERROR "Could not find slang")
   		ENDIF (SLANG_FIND_REQUIRED)
	ENDIF (SLANG_FOUND)
ENDIF (SLANG_LIBRARY AND SLANG_INCLUDE_DIR)
