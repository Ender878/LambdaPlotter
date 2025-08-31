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
file(GLOB_RECURSE LAMBDAPLOTTER CONFIGURE_DEPENDS "./src/LP/*.*")
file(GLOB_RECURSE IMPLOT CONFIGURE_DEPENDS "./src/implot/*.*")
file(GLOB_RECURSE BINDINGS CONFIGURE_DEPENDS "./src/bindings/*.*")
file(GLOB_RECURSE FONTS CONFIGURE_DEPENDS "./src/fonts/*.cpp")

# --- Find Packages ---
find_package(imgui REQUIRED)
find_package(glfw3 REQUIRED)
find_package(glad  REQUIRED)

include(FetchContent)
FetchContent_Declare(nfd
    GIT_REPOSITORY "https://github.com/btzy/nativefiledialog-extended.git"
    GIT_TAG "origin/master"   # it's much better to use a specific Git revision or Git tag for reproducibility
    SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}/include/nfd"
)
FetchContent_MakeAvailable(nfd)

add_library(lplib STATIC
    ${LAMBDAPLOTTER}
    ${IMPLOT}
    ${BINDINGS}
    ${FONTS}
)

target_include_directories(lplib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(lplib PUBLIC imgui::imgui glfw glad::glad nfd)

add_executable(lambda_plotter
    src/main.cpp 
)

target_link_libraries(lambda_plotter PRIVATE lplib)

# --- Compiler Flags ---
target_compile_options(lplib PRIVATE
    # Debug flags
    $<$<AND:$<CONFIG:Debug>,$<CXX_COMPILER_ID:GNU,Clang>>:-g>
    
    # Release flags
    $<$<AND:$<CONFIG:Release>,$<CXX_COMPILER_ID:GNU,Clang>>:-O3>
    $<$<AND:$<CONFIG:Release>,$<CXX_COMPILER_ID:MSVC>>:/Ox>
    $<$<CONFIG:Release>:-DNDEBUG>
    
    # Warnings and other flags
    $<$<CXX_COMPILER_ID:GNU,Clang>:-Wall -Wextra -Wpedantic>
    $<$<CXX_COMPILER_ID:MSVC>:/W4>
    $<$<CXX_COMPILER_ID:MSVC>:/utf-8>
)

# --- App icons ---
if(WIN32)
    target_sources(lambda_plotter PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/assets/icon.rc)
elseif(APPLE)
    # Configure the application bundle properties.
    set_target_properties(lambda_plotter PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_ICON_FILE assets/icon.icns
        MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/assets/Info.plist"
    )
endif()

# --- Testing ---
if(LP_BUILD_TESTS)
    enable_testing()
    find_package(GTest REQUIRED)
    include(GoogleTest)

    add_executable(lp_tests tests/telemetry_tests.cpp)
    target_link_libraries(lp_tests PRIVATE lplib GTest::gtest GTest::gtest_main)

    gtest_discover_tests(lp_tests)
endif()
