add_library(semantics "")
target_include_directories(semantics PUBLIC ${MINUET_LANG_SRC_DIR})
target_sources(semantics PRIVATE analyzer.cpp)
