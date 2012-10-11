cmake_minimum_required( VERSION 2.8 )

project( g-signal-collector )

find_package( PkgConfig )
pkg_check_modules( GOBJECT REQUIRED gobject-2.0 )
include_directories( ${GOBJECT_INCLUDE_DIRS} )
link_directories( ${GOBJECT_LINK_DIRS} )

add_subdirectory( src )

add_subdirectory( test )
