cmake_minimum_required(VERSION 3.20.0)
project(lambda_plotter VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# --- Build Configuration ---
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type (Release, Debug, etc..)")
endif()

option(LP_BUILD_TESTS "Build the tests for this project" ON)

# --- Gather source files ---
file(GLOB LAMBDAPLOTTER "./src/LP/*.*")
file(GLOB IMPLOT "./src/implot/*.*")
file(GLOB BINDINGS "./src/bindings/*.*")
file(GLOB FONTS "./src/fonts/*.c")
file(GLOB TINYFD "./src/tinyfd/*.c")

# --- Find Packages ---
find_package(imgui REQUIRED)
find_package(glfw3 REQUIRED)
find_package(glad  REQUIRED)

add_library(lplib STATIC
    ${LAMBDAPLOTTER}
    ${IMPLOT}
    ${BINDINGS}
    ${FONTS}
    ${TINYFD}
)

target_include_directories(lplib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(lplib PUBLIC imgui::imgui glfw glad::glad)

add_executable(lambda_plotter
    src/main.cpp 
)

target_link_libraries(lambda_plotter PRIVATE lplib)

# --- Compiler Flags ---
target_compile_options(lplib PRIVATE
    $<$<CONFIG:Debug>:-g>
    $<$<CONFIG:Release>:-O3 -DNDEBUG>
    $<$<CXX_COMPILER_ID:GNU,Clang>:-Wall -Wextra -Wpedantic>
    $<$<CXX_COMPILER_ID:MSVC>:/utf-8>
)

# --- Testing ---
if(LP_BUILD_TESTS)
    enable_testing()
    find_package(GTest REQUIRED)
    include(GoogleTest)

    add_executable(lp_tests tests/telemetry_tests.cpp)
    target_link_libraries(lp_tests PRIVATE lplib GTest::gtest GTest::gtest_main)

    gtest_discover_tests(lp_tests)
endif()
