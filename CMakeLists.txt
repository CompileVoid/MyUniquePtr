cmake_minimum_required(VERSION 3.25)
project(MyUniquePtrProject LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(MyUniquePtr INTERFACE)
target_include_directories(MyUniquePtr INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

find_package(GTest CONFIG REQUIRED)

enable_testing()

add_executable(run_tests tests/test_my_unique_ptr.cpp)
target_link_libraries(run_tests PRIVATE MyUniquePtr GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(run_tests)