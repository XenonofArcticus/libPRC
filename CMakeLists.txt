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

#
# Select asymptote (for dev and verification) or native libPRC.
option( PRC_USE_ASYMPTOTE "Set to ON to use Asymptote instead of libPRC (testing/dev only)." ON )
if( PRC_USE_ASYMPTOTE )
    add_definitions( -DPRC_USE_ASYMPTOTE )
    set( PRC_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/src/asymptote )
    set( PRC_LIBRARY asymptote )
else()
    set( PRC_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/src/libPRC )
    set( PRC_LIBRARY libPRC )
endif()


add_subdirectory( src )
