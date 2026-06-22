set(IMGUI_DIR "../../third_party/imgui/")

file(GLOB SOURCES CONFIGURE_DEPENDS "${IMGUI_DIR}/*.cpp")

add_compile_definitions(
    GLFW_INCLUDE_NONE
)

add_library(imgui STATIC 
    ${SOURCES}
    ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
    ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
    ${IMGUI_DIR}/backends/imgui_impl_opengl3_loader.h
)
target_link_libraries(imgui glfw glad::glad)
target_include_directories(imgui PUBLIC ${IMGUI_DIR} ${IMGUI_DIR}/backends)
