cmake_minimum_required(VERSION 3.20.0)
project(better_serial_plotter VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# --- Build Configuration ---
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type (Release, Debug, etc..)")
endif()

option(BSP_BUILD_TESTS "Build the tests for this project" ON)

# --- Find Packages ---
find_package(imgui REQUIRED)
find_package(glfw3 REQUIRED)
find_package(glew  REQUIRED)

add_library(bsplib STATIC
    src/BSP/telemetry.cpp
    src/BSP/serial.cpp
    src/BSP/controller.cpp
    src/BSP/toolbar.cpp
    src/BSP/plotView.cpp
    src/BSP/window.cpp
    src/common/shared.cpp
)

target_include_directories(bsplib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(bsplib PUBLIC imgui::imgui glfw GLEW::GLEW)

add_executable(better_serial_plotter
    src/main.cpp 
    src/bindings/imgui_impl_opengl3.cpp 
    src/bindings/imgui_impl_glfw.cpp 
    src/bindings/imgui_impl_opengl3_loader.h 
    src/implot/implot.cpp
    src/implot/implot_items.cpp
    src/tinyfd/tinyfiledialogs.c
    src/fonts/lucide.c
    src/fonts/roboto.c
)

target_link_libraries(better_serial_plotter PRIVATE bsplib)

# --- Compiler Flags ---
target_compile_options(better_serial_plotter PRIVATE
    $<$<CONFIG:Debug>:-g>
    $<$<CONFIG:Release>:-O3 -DNDEBUG>
    $<$<CXX_COMPILER_ID:GNU,Clang>:-Wall -Wextra -Wpedantic>
)

# --- Testing ---
if(BSP_BUILD_TESTS)
    enable_testing()
    find_package(GTest REQUIRED)
    include(GoogleTest)

    add_executable(bsp_test tests/bsp_tests.cpp)
    target_link_libraries(bsp_test PRIVATE bsplib GTest::gtest GTest::gtest_main)

    gtest_discover_tests(bsp_test)
endif()
