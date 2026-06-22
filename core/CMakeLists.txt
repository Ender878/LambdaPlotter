file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS "*")

add_library(core ${SOURCES})

add_compile_definitions(
    GLFW_INCLUDE_NONE
)

target_link_libraries(core imgui glm::glm)
target_include_directories(core PUBLIC .)
