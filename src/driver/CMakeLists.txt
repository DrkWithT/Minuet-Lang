add_library(driver "")
target_include_directories(driver PUBLIC ${MINUET_LANG_SRC_DIR})
target_sources(driver PRIVATE ./plugins/tac_condenser.cpp PRIVATE ./plugins/ir_dumper.cpp PRIVATE ./plugins/disassembler.cpp PRIVATE sources.cpp PRIVATE driver.cpp)
