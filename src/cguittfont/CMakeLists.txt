# Alas, CGUITTFont.{cpp,h} don't include irrUString.h even though they use
# core::ustring.  This solution is fugly and only works with GCC, but the
# official "recommendation" would be that every user that builds minetest
# from source has to copy irrUString.h to her Irrlicht include dir and
# add an #include line to irrlicht.h.
# We also don't want to (and shouldn't have to) modify our copies of
# CGUITTFont.{cpp,h} because that would complicate updating them later.
if(CMAKE_COMPILER_IS_GNUCXX)
	add_definitions(-include "${CMAKE_CURRENT_SOURCE_DIR}/irrUString.h")
	add_definitions(-include "stddef.h")
endif(CMAKE_COMPILER_IS_GNUCXX)

include_directories(
	${IRRLICHT_INCLUDE_DIR}
	${FREETYPE_INCLUDE_DIRS}
)

add_library(cguittfont CGUITTFont.cpp)

target_link_libraries(
	cguittfont
	${IRRLICHT_LIBRARY}
	${FREETYPE_LIBRARIES}
)
