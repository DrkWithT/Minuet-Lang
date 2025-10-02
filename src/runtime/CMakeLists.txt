add_library(runtime "")
target_include_directories(runtime PUBLIC ${MINUET_LANG_SRC_DIR})
target_sources(runtime PRIVATE value.cpp)
