project(cloudy)
cmake_minimum_required(VERSION 2.6)

find_package(CGAL REQUIRED)
find_package(PythonLibs REQUIRED)
find_package(Boost COMPONENTS python REQUIRED)
find_package(Qt4 REQUIRED)

set(QT_USE_QTOPENGL 1)
include(${CGAL_USE_FILE})
include_directories(${PYTHON_INCLUDE_PATH})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${QT_INCLUDES})

link_directories(${CMAKEDEMO_BINARY_DIR}/cloudy)
link_libraries(${PYTHON_LIBRARIES})
link_libraries(${Boost_LIBRARIES})

add_subdirectory(cloudy)
add_subdirectory(bindings)
add_subdirectory(tools)


