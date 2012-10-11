add_library(
	g-signal-collector SHARED
	collector
	signal-info
	signal-info-pool
	signal-info-pool-dumper
)

target_link_libraries( g-signal-collector ${GOBJECT_LIBRARIES} )

install( TARGETS g-signal-collector LIBRARY DESTINATION lib )

