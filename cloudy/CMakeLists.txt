#add_subdirectory(offset)
#add_subdirectory(misc)
#add_subdirectory(view)

set(QT_USE_QTOPENGL 1)
set(QT_USE_QTXML 1)
include(${QT_USE_FILE})

set(cloudy_SRCS 
  "Cloud.cpp"
  "linear/Linear.cpp"
  "linear/Covariance.cpp"
  "misc/Program_options.cpp"
  "mesh/Mesh.cpp"
  "mesh/Gradient.cpp"
  "view/Widget.cpp"
  "view/Director.cpp")
set(cloudy_MOC_HDR
  "view/Widget.hpp"
  "view/Editor.hpp")
QT4_WRAP_CPP(cloudy_SRCS ${cloudy_MOC_HDR})

add_library(cloudy SHARED ${cloudy_SRCS})
target_link_libraries(cloudy ${QT_LIBRARIES})
target_link_libraries(cloudy ANN)