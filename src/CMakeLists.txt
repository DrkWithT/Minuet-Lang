add_library(frontend "")
target_sources(frontend PRIVATE frontend/lexing.cpp PRIVATE frontend/parsing.cpp)

add_library(ir "")
target_sources(ir PRIVATE ir/steps.cpp PRIVATE ir/cfg.cpp PRIVATE ir/printing.cpp PRIVATE ir/convert_ast.cpp)

add_library(codegen "")
target_sources(codegen PRIVATE codegen/disassemble.cpp PRIVATE codegen/emitter.cpp)

add_library(driver "")
target_sources(driver PRIVATE driver/sources.cpp PRIVATE driver/driver.cpp)

add_library(runtime "")
target_sources(runtime PRIVATE runtime/value.cpp PRIVATE runtime/bytecode.cpp PRIVATE runtime/vm.cpp)

add_executable(minuetm main.cpp)
target_link_directories(minuetm PRIVATE ${MINUET_LANG_LIB_DIR})
target_link_libraries(minuetm PRIVATE frontend PRIVATE ir PRIVATE codegen PRIVATE driver PRIVATE runtime)
