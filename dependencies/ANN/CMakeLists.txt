add_definitions(-DDLL_EXPORTS)

set(ann_SRCS src/ANN.cpp src/bd_fix_rad_search.cpp
src/bd_pr_search.cpp src/bd_search.cpp src/bd_tree.cpp src/brute.cpp
src/kd_dump.cpp src/kd_fix_rad_search.cpp src/kd_pr_search.cpp
src/kd_search.cpp src/kd_split.cpp src/kd_tree.cpp src/kd_util.cpp
src/perf.cpp)

add_library(ANN SHARED ${ann_SRCS})
