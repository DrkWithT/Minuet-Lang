add_library(mintrinsics "")
target_include_directories(mintrinsics PUBLIC ${MINUET_LANG_SRC_DIR})
target_sources(mintrinsics PRIVATE mnl_stdio.cpp PRIVATE mnl_lists.cpp)
