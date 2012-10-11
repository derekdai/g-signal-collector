cmake_minimum_required( VERSION 2.8 )

project( g-signal-collector )

find_package( PkgConfig )
pkg_check_modules( GOBJECT REQUIRED gobject-2.0 )
include_directories( ${GOBJECT_INCLUDE_DIRS} )
link_directories( ${GOBJECT_LINK_DIRS} )

add_library(
	g-signal-collector SHARED
	collector
	signal-info
	signal-info-pool
	signal-info-pool-dumper
)

target_link_libraries( g-signal-collector ${GOBJECT_LIBRARIES} )

install( TARGETS g-signal-collector LIBRARY DESTINATION lib )
