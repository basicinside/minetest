How to update the bundled Freetype:

1. Unpack the new freetype source tarball in this directory, so that a
   subdirectory called freetype-<version> is created.
2. Delete the subdirectory that belongs to the old freetype version.
3. Update FREETYPE_BUNDLE_VERSION in
   ../../cmake/Modules/FindFreetypeBundled.cmake
4. Check freetype-<version>/docs/INSTALL.ANY if any changes to
   CMakeLists.txt have to be made.
5. Test.

Of course, major changes to the freetype directory structure or its
build system may demand additional steps.
