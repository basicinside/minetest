# Look for freetype, use our own if not found
# This may not be called "FindFreetype.cmake" because such a module already
# exists in vanilla cmake, and is in fact called from here to look for
# the system freetype.

SET(FREETYPE_BUNDLE_VERSION 2.4.7)
SET(FREETYPE_BUNDLE_DIR ${PROJECT_SOURCE_DIR}/freetype/freetype-${FREETYPE_BUNDLE_VERSION})

FIND_PACKAGE(Freetype)

IF(FREETYPE_FOUND)
	MESSAGE(STATUS "Found system freetype header file in ${FREETYPE_INCLUDE_DIR_ft2build}")
	MESSAGE(STATUS "Found system freetype2 header file in ${FREETYPE_INCLUDE_DIR_freetype2}")
	MESSAGE(STATUS "Found system freetype library ${FREETYPE_LIBRARY}")
ELSE(FREETYPE_FOUND)
	SET(FREETYPE_INCLUDE_DIR_ft2build ${FREETYPE_BUNDLE_DIR}/include)
	SET(FREETYPE_INCLUDE_DIR_freetype2 ${FREETYPE_BUNDLE_DIR}/include)
	SET(FREETYPE_INCLUDE_DIRS ${FREETYPE_BUNDLE_DIR}/include)
	SET(FREETYPE_LIBRARY freetype)
	MESSAGE(STATUS "Using project freetype library")
ENDIF(FREETYPE_FOUND)
