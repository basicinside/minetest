# See freetype-2.X.X/docs/INSTALL.ANY.

set(freetype_SOURCE_DIR freetype-${FREETYPE_BUNDLE_VERSION}/src)

if(WIN32)
	# Windows
	set(freetype_platform_SRCS "")
	set(freetype_platform_LIBS "")
else(WIN32)
	# Unix probably
	if(APPLE)
		# Mac OS X
		set(freetype_platform_SRCS
			${freetype_SOURCE_DIR}/base/ftmac.c
		)
		set(freetype_platform_LIBS "")
	else(APPLE)
		# other Unix
		set(freetype_platform_SRCS "")
		set(freetype_platform_LIBS "")
	endif(APPLE)
endif(WIN32)

include_directories(${ZLIB_INCLUDE_DIR})

add_library(freetype
	# base components (required)
	${freetype_SOURCE_DIR}/base/ftsystem.c
	${freetype_SOURCE_DIR}/base/ftinit.c
	${freetype_SOURCE_DIR}/base/ftdebug.c
	${freetype_SOURCE_DIR}/base/ftbase.c
	${freetype_SOURCE_DIR}/base/ftbbox.c
	${freetype_SOURCE_DIR}/base/ftglyph.c
	${freetype_SOURCE_DIR}/base/ftbdf.c
	${freetype_SOURCE_DIR}/base/ftbitmap.c
	${freetype_SOURCE_DIR}/base/ftcid.c
	${freetype_SOURCE_DIR}/base/ftfstype.c
	${freetype_SOURCE_DIR}/base/ftgasp.c
	${freetype_SOURCE_DIR}/base/ftgxval.c
	${freetype_SOURCE_DIR}/base/ftlcdfil.c
	${freetype_SOURCE_DIR}/base/ftmm.c
	${freetype_SOURCE_DIR}/base/ftotval.c
	${freetype_SOURCE_DIR}/base/ftpatent.c
	${freetype_SOURCE_DIR}/base/ftpfr.c
	${freetype_SOURCE_DIR}/base/ftstroke.c
	${freetype_SOURCE_DIR}/base/ftsynth.c
	${freetype_SOURCE_DIR}/base/fttype1.c
	${freetype_SOURCE_DIR}/base/ftwinfmt.c
	${freetype_SOURCE_DIR}/base/ftxf86.c
	#${freetype_SOURCE_DIR}/base/ftmac.c  # only on the Macintosh

	# font drivers (optional; at least one is needed)
	${freetype_SOURCE_DIR}/bdf/bdf.c
	${freetype_SOURCE_DIR}/cff/cff.c
	${freetype_SOURCE_DIR}/cid/type1cid.c
	${freetype_SOURCE_DIR}/pcf/pcf.c
	${freetype_SOURCE_DIR}/pfr/pfr.c
	${freetype_SOURCE_DIR}/sfnt/sfnt.c
	${freetype_SOURCE_DIR}/truetype/truetype.c
	${freetype_SOURCE_DIR}/type1/type1.c
	${freetype_SOURCE_DIR}/type42/type42.c
	${freetype_SOURCE_DIR}/winfonts/winfnt.c

	# rasterizers (optional; at least one is needed for vector formats)
	${freetype_SOURCE_DIR}/raster/raster.c
	${freetype_SOURCE_DIR}/smooth/smooth.c

	# auxiliary modules (optional)
	${freetype_SOURCE_DIR}/autofit/autofit.c
	${freetype_SOURCE_DIR}/cache/ftcache.c
	${freetype_SOURCE_DIR}/gzip/ftgzip.c  # needs zlib or uses bundled
	${freetype_SOURCE_DIR}/lzw/ftlzw.c
	#${freetype_SOURCE_DIR}/bzip2/ftbzip2.c  # needs bzlib
	#${freetype_SOURCE_DIR}/gxvalid/gxvalid.c
	#${freetype_SOURCE_DIR}/otvalid/otvalid.c
	${freetype_SOURCE_DIR}/psaux/psaux.c
	${freetype_SOURCE_DIR}/pshinter/pshinter.c
	${freetype_SOURCE_DIR}/psnames/psnames.c

	${freetype_platform_SRCS}
)

target_link_libraries(
	freetype
	${ZLIB_LIBRARIES}
	${freetype_platform_LIBS}
)
