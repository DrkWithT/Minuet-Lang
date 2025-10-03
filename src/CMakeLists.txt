add_library(frontend "")
target_include_directories(frontend PUBLIC ${MINUET_LANG_SRC_DIR})
target_sources(frontend PRIVATE frontend/lexing.cpp PRIVATE frontend/parsing.cpp)

add_library(ir "")
target_include_directories(ir PRIVATE ${MINUET_LANG_SRC_DIR})
target_sources(ir PRIVATE ir/steps.cpp PRIVATE ir/cfg.cpp PRIVATE ir/printing.cpp PRIVATE ir/convert_ast.cpp)

add_library(driver "")
target_include_directories(driver PUBLIC ${MINUET_LANG_SRC_DIR})
target_sources(driver PRIVATE driver/sources.cpp PRIVATE driver/driver.cpp)

add_library(runtime "")
target_include_directories(runtime PUBLIC ${MINUET_LANG_SRC_DIR})
target_sources(runtime PRIVATE runtime/value.cpp)


add_executable(minuetm main.cpp)
target_link_directories(minuetm PRIVATE ${MINUET_LANG_LIB_DIR})
target_link_libraries(minuetm PRIVATE frontend PRIVATE ir PRIVATE driver PRIVATE runtime)
target_include_directories(minuetm PUBLIC ${MINUET_LANG_SRC_DIR})
