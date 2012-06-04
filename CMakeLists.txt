project(cloudy)
cmake_minimum_required(VERSION 2.4.5)

set(CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS true)
if (COMMAND cmake_policy)
  cmake_policy(SET CMP0003 NEW)
endif()

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake/modules)

find_package(CGAL QUIET COMPONENTS Core)
#find_package(PythonLibs REQUIRED)
find_package(Boost REQUIRED)
#find_package(IETL REQUIRED)
find_package(Qt4 REQUIRED)
#find_package(GSL REQUIRED)

set(CMAKE_CXX_FLAGS -fopenmp)

set(QT_USE_QTOPENGL 1)
include(${CGAL_USE_FILE})
#include_directories(${PYTHON_INCLUDE_PATH})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/dependencies/boost-bindings)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/dependencies/ANN/include)
include_directories(${QT_INCLUDES})
include_directories(${IETL_INCLUDE_DIRS})

link_directories(${CMAKEDEMO_BINARY_DIR}/cloudy)
link_directories(${CMAKEDEMO_BINARY_DIR}/dependencies/ANN)
#link_libraries(${PYTHON_LIBRARIES})
link_libraries(${Boost_LIBRARIES})
#link_libraries(gsl) #${GSL_LIBRARIES})
link_libraries(lapack)
link_libraries(${QT_QTOPENGL_LIBRARY})
link_libraries(GL)
link_libraries(GLU)
add_subdirectory(dependencies)
add_subdirectory(cloudy)
#add_subdirectory(bindings)
add_subdirectory(experiments)
add_subdirectory(tools)


