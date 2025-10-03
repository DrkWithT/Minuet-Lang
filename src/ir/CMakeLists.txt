add_library(ir "")
target_include_directories(ir PRIVATE ${MINUET_LANG_SRC_DIR})
target_sources(ir PRIVATE steps.cpp PRIVATE cfg.cpp PRIVATE printing.cpp PRIVATE convert_ast.cpp)
