add_library(runtime "")
target_include_directories(runtime PUBLIC ${MINUET_LANG_SRC_DIR})
target_sources(runtime PRIVATE fast_value.cpp PRIVATE sequence_value.cpp PRIVATE heap_storage.cpp PRIVATE bytecode.cpp PRIVATE vm.cpp)
