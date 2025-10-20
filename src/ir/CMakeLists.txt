add_library(ir "")
target_include_directories(ir PUBLIC ${MINUET_LANG_SRC_DIR})
target_sources(ir PRIVATE steps.cpp PRIVATE cfg.cpp PRIVATE convert_ast.cpp)
