file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS "src/*")

add_compile_definitions(
    GLFW_INCLUDE_NONE
)

add_library(imgui STATIC ${SOURCES})
target_link_libraries(imgui glfw glad::glad)
target_include_directories(imgui PUBLIC src src/backend)
