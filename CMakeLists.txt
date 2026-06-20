cmake_minimum_required(VERSION 4.0)
project(lambda_plotter CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(glad REQUIRED)
find_package(glfw3 REQUIRED)
find_package(OpenGL REQUIRED)

add_subdirectory(app)
add_subdirectory(imgui)
