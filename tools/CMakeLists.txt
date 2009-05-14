add_executable(pctoffset pctoffset.cpp)
target_link_libraries(pctoffset cloudy)

set(QT_USE_QTOPENGL 1)
set(QT_USE_QTXML 1)
include(${QT_USE_FILE})

add_executable(pctviewer pctviewer.cpp)
target_link_libraries(pctviewer cloudy)
target_link_libraries(pctviewer ${QT_LIBRARIES})
