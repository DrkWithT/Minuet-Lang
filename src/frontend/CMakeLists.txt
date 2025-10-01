add_library(frontend "")
target_include_directories(frontend PUBLIC ${MINUET_LANG_SRC_DIR})
target_sources(frontend PRIVATE lexing.cpp PRIVATE parsing.cpp)
