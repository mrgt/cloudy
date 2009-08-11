add_executable(pctoffset pctoffset.cpp)
target_link_libraries(pctoffset cloudy)

add_executable(pctoffsetmc pctoffsetmc.cpp)
target_link_libraries(pctoffsetmc cloudy)

add_executable(pctconvolve pctconvolve.cpp)
target_link_libraries(pctconvolve cloudy)

add_executable(pctkdistance pctkdistance.cpp)
target_link_libraries(pctkdistance cloudy)

set(QT_USE_QTOPENGL 1)
set(QT_USE_QTXML 1)
include(${QT_USE_FILE})

add_executable(pcvcloud pcvcloud.cpp)
target_link_libraries(pcvcloud cloudy ${QT_LIBRARIES})

add_executable(pcvcovariance pcvcovariance.cpp)
target_link_libraries(pcvcovariance cloudy ${QT_LIBRARIES})
