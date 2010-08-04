add_executable(pctoffset pctoffset.cpp)
target_link_libraries(pctoffset cloudy)

add_executable(pctoffsetEX pctoffsetEX.cpp)
target_link_libraries(pctoffsetEX cloudy)

add_executable(pctoffsetmc pctoffsetmc.cpp)
target_link_libraries(pctoffsetmc cloudy)

add_executable(pctconvolve pctconvolve.cpp)
target_link_libraries(pctconvolve cloudy)

add_executable(pctnoisify pctnoisify.cpp)
target_link_libraries(pctnoisify cloudy)

add_executable(pctoldcov2cov pctoldcov2cov.cpp)
target_link_libraries(pctoldcov2cov cloudy)

add_executable(pctkdistance pctkdistance.cpp)
target_link_libraries(pctkdistance cloudy)

add_executable(pctoffcolorize offcolorize.cpp)
target_link_libraries(pctoffcolorize cloudy)

add_executable(pctoffcolorizenn pctoffcolorizenn.cpp)
target_link_libraries(pctoffcolorizenn cloudy)

add_executable(pctoffextract pctextract.cpp)
target_link_libraries(pctoffextract cloudy)

add_executable(pctoffnormalize pctoffnormalize.cpp)
target_link_libraries(pctoffnormalize cloudy)

add_executable(pctrandomcloud pctrandomcloud.cpp)
target_link_libraries(pctrandomcloud cloudy)

set(QT_USE_QTOPENGL 1)
set(QT_USE_QTXML 1)
include(${QT_USE_FILE})

add_executable(pcvcloud pcvcloud.cpp)
target_link_libraries(pcvcloud cloudy ${QT_LIBRARIES})

add_executable(pcvmatching pcvmatching.cpp)
target_link_libraries(pcvmatching cloudy ${QT_LIBRARIES})

#add_executable(pcvwcloud-balls pcvwcloud-balls.cpp)
#target_link_libraries(pcvwcloud-balls cloudy ${QT_LIBRARIES})

add_executable(pcvmesh pcvmesh.cpp)
target_link_libraries(pcvmesh cloudy ${QT_LIBRARIES})

add_executable(pcvcovariance pcvcovariance.cpp)
target_link_libraries(pcvcovariance cloudy ${QT_LIBRARIES})
