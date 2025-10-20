cmake_minimum_required(VERSION 3.30)

project(minuetlang VERSION 0.1.0 LANGUAGES CXX)

set(MINUET_LANG_SRC_DIR ${CMAKE_SOURCE_DIR}/src/)
# set(MINUET_TESTS_DIR ${CMAKE_SOURCE_DIR}/tests)
set(MINUET_LANG_LIB_DIR ${CMAKE_SOURCE_DIR}/build)
set(MINUET_LANG_DEMO_DIR ${CMAKE_SOURCE_DIR}/test_suite)

if (DEFINED MY_FLAGS)
    add_compile_options(${MY_FLAGS})
else ()
    message(FATAL_ERROR "Preset invalid because it is missing the MY_FLAGS variable.")
endif ()


add_subdirectory(${MINUET_LANG_SRC_DIR})
# enable_testing()
# add_subdirectory(${MINUET_LANG_UNIT_TEST_DIR})
