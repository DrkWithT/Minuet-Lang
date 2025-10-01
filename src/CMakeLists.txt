add_subdirectory(frontend)
add_subdirectory(driver)

add_executable(minuetm main.cpp)
target_link_directories(minuetm PRIVATE ${MINUET_LANG_LIB_DIR})
target_link_libraries(minuetm PRIVATE frontend PRIVATE driver)
target_include_directories(minuetm PUBLIC ${MINUET_LANG_SRC_DIR})
