file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS "*.cpp")

add_executable(lambda_plotter ${SOURCES})

add_compile_definitions(
    GLFW_INCLUDE_NONE
)

target_link_libraries(lambda_plotter core)
target_include_directories(lambda_plotter PRIVATE .)
