#
# libPRC top-level CMakeLists.txt
# https://github.com/XenonofArcticus/libPRC
#

cmake_minimum_required( VERSION 2.8.5 )
project( libPRC )

# Define project-specific macros.
set( CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/CMakeModules ${CMAKE_MODULE_PATH} )
include( CMakeMacros )



#
# Required ZLIB dependency.
# Set ZLIB_ROOT (env or cmake) as hint, if necessary.
find_package( ZLIB REQUIRED )

#
# Optional OpenSceneGraph. If not found, the osgdb_prc plugin
# library will not be built.
# Set OSG_DIR (env or cmake) as hint, if necessary.
set( _osgComponents osgGA osgText osgViewer osgSim osgDB osgUtil osg OpenThreads )
find_package( OpenSceneGraph 2.8.5 COMPONENTS ${_osgComponents} )


add_subdirectory( src )
