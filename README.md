libPRC
======

PRC file format library


Dependencies
============

 - zlib (set ZLIB_ROOT as a hint, if necessary)
 - OpenSceneGraph (set OSG_DIR as a hint, if necessary)
 - libHaru (set LIBHARU_ROOT as a hint, if necessary)
 - libPNG (aet PNG_PNG_INCLUDE_DIR and PNG_LIBRARY, if necessary)
 - Doxygen

OpenSceneGraph is required to build the osgdb_prc export plugin and
the prcconvert executable file conversion tool.

libHaru, libPNG, and zlib are required to build the prctopdf
executable that embeds PRC files in PDF documents.

Doxygen is required to build documentation from the source.
Set LIBPRC_DOCUMENTATION to ON to do this.


Using CMake
===========

You could run cmake-gui (or ccmake) and fill in the necessary
variables so that CMake can find the dependencies.

Optionally you can run cmake from the command line with something
like this:

> cmake <path_to_source> \
      -DOSG_DIR=<path_to_osg> \
      -DZLIB_ROOT=<path_to_zlib> \
      -DLIBHARU_ROOT=<path_to_libharu> \
      -DPNG_PNG_INCLUDE_DIR=<png_include_dir> \
      -DPNG_LIBRARY=<png_library>
