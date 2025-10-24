add_library(bcgen "")
target_include_directories(bcgen PUBLIC ${MINUET_LANG_SRC_DIR})
target_sources(bcgen PRIVATE emitter.cpp)
