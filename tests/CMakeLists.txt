cmake_minimum_required(VERSION 3.14)
project(iso14229_tests CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)

set(CMAKE_POLICY_DEFAULT_CMP0135 NEW)

set(gtest_force_shared_ctr ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

add_library(iso14229 STATIC ../iso14229_uds_practice/iso14229.c)
target_include_directories(iso14229 PUBLIC ../iso14229_uds_practice)
target_link_libraries(iso14229 PUBLIC pthread)

enable_testing()
add_executable(run_tests test_main.cpp)
target_link_libraries(run_tests PRIVATE gtest_main iso14229)      
target_include_directories(run_tests PRIVATE ../iso14229_uds_practice)

include(GoogleTest)
gtest_discover_tests(run_tests)