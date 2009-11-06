
include(${CGAL_USE_FILE})
add_library(pycloudy MODULE "offset.cpp")
set_target_properties(pycloudy PROPERTIES PREFIX "")

