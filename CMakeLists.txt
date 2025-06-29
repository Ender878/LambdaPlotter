cmake_minimum_required(VERSION 3.20.0)
project(better_serial_plotter VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

enable_testing()

find_package(imgui REQUIRED)
find_package(glfw3 REQUIRED)
find_package(glew REQUIRED)

add_executable(better_serial_plotter
    src/main.cpp 
    src/bindings/imgui_impl_opengl3.cpp 
    src/bindings/imgui_impl_glfw.cpp 
    src/bindings/imgui_impl_opengl3_loader.h 
    src/implot/implot.cpp
    src/implot/implot_items.cpp
    src/BSP/window.cpp
    src/BSP/serial.cpp
    src/BSP/controller.cpp
    src/BSP/toolbar.cpp
    src/BSP/telemetry.cpp
    src/common/shared.cpp
    src/tinyfd/tinyfiledialogs.c
)

target_link_libraries(better_serial_plotter PRIVATE imgui::imgui glfw GLEW::GLEW)

find_package(GTest REQUIRED)

add_executable(bsp_test tests/bsp_tests.cpp)
target_link_libraries(bsp_test gtest)

add_test(NAME BSP_Test COMMAND bsp_test)
