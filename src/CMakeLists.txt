add_subdirectory(frontend)
add_subdirectory(semantics)
add_subdirectory(ir)
add_subdirectory(bcgen)
add_subdirectory(driver)
add_subdirectory(runtime)
add_subdirectory(mintrinsics)


add_executable(minuetm main.cpp)
target_include_directories(minuetm PUBLIC ${MINUET_LANG_SRC_DIR})
target_link_directories(minuetm PRIVATE ${MINUET_LANG_LIB_DIR})
target_link_libraries(minuetm PRIVATE frontend PRIVATE semantics PRIVATE ir PRIVATE bcgen PRIVATE driver PRIVATE runtime PRIVATE mintrinsics)
