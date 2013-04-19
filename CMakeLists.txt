cmake_minimum_required( VERSION 2.8 )

project( g-signal-collector )

find_package( PkgConfig )
pkg_check_modules( GIO REQUIRED gio-2.0 )
include_directories( ${GIO_INCLUDE_DIRS} )
link_directories( ${GIO_LINK_DIRS} )

add_subdirectory( src )

add_subdirectory( test )
