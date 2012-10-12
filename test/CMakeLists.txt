pkg_check_modules( CLUTTER REQUIRED clutter-1.0 )
include_directories( ${CLUTTER_INCLUDE_DIRS} )
link_directories( ${CLUTTER_LIBRARY_DIRS} )

add_executable( clutter-rectangles clutter-rectangles )
target_link_libraries( clutter-rectangles ${CLUTTER_LIBRARIES} )

pkg_check_modules( GTK REQUIRED gtk+-3.0 )
include_directories( ${GTK_INCLUDE_DIRS} )
link_directories( ${GTK_LIBRARY_DIRS} )

add_executable( gtk-rectangles gtk-rectangles )
target_link_libraries( gtk-rectangles ${GTK_LIBRARIES} )
