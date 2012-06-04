set(QT_USE_QTOPENGL 1)
set(QT_USE_QTXML 1)
include(${QT_USE_FILE})

add_executable(torus torus.cpp)
target_link_libraries(torus cloudy ${QT_LIBRARIES})

add_executable(gentorus gentorus.cpp)
target_link_libraries(gentorus cloudy)

