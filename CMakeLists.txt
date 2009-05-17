project(cloudy)
cmake_minimum_required(VERSION 2.4.5)

set(CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS true)
if (COMMAND cmake_policy)
  cmake_policy(SET CMP0003 NEW)
endif()
 
find_package(CGAL QUIET COMPONENTS Core)
find_package(PythonLibs REQUIRED)
find_package(Boost COMPONENTS python REQUIRED)
find_package(Qt4 REQUIRED)

set(QT_USE_QTOPENGL 1)
include(${CGAL_USE_FILE})
include_directories(${PYTHON_INCLUDE_PATH})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/dependencies/boost-bindings)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/dependencies/ANN/include)
include_directories(${QT_INCLUDES})

link_directories(${CMAKEDEMO_BINARY_DIR}/cloudy)
link_directories(${CMAKEDEMO_BINARY_DIR}/dependencies/ANN)
link_libraries(${PYTHON_LIBRARIES})
link_libraries(${Boost_LIBRARIES})
link_libraries(lapack)

add_subdirectory(dependencies)
add_subdirectory(cloudy)
#add_subdirectory(bindings)
add_subdirectory(tools)


