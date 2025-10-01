add_library(driver "")
target_include_directories(driver PUBLIC ${MINUET_LANG_SRC_DIR})
target_sources(driver PRIVATE sources.cpp PRIVATE driver.cpp)
