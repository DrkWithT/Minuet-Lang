add_library(codegen "")
target_include_directories(codegen PUBLIC ${MINUET_LANG_SRC_DIR})
target_sources(codegen PRIVATE emitter.cpp)
