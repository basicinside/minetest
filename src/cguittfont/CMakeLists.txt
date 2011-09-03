# Alas, CGUITTFont.{cpp,h} don't include irrUString.h even though they use
# core::ustring.  This solution is fugly and only works with GCC, but the
# official "recommendation" would be that every user (that includes people
# who install Minetest binaries without bundled Irrlicht!) copies irrUString.h
# to her Irrlicht include dir and modifies irrlicht.h.
# We also don't want to (and shouldn't have to) modify our copies of
# CGUITTFont.{cpp,h} because that would complicate updating them later.
if(CMAKE_COMPILER_IS_GNUCXX)
	add_definitions(-include "${CMAKE_CURRENT_SOURCE_DIR}/irrUString.h")
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
