set(IMGUI_DIR "../../third_party/imgui/")
set(IMPLOT_DIR "../../third_party/implot/")

file(GLOB IMGUI_SOURCES CONFIGURE_DEPENDS "${IMGUI_DIR}/*.cpp")
file(GLOB IMPLOT_SOURCES CONFIGURE_DEPENDS "${IMPLOT_DIR}/*.cpp")

add_compile_definitions(
    GLFW_INCLUDE_NONE
)

add_library(imgui STATIC 
    ${IMGUI_SOURCES}
    ${IMPLOT_SOURCES}
    ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
    ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
)
target_link_libraries(imgui glfw glad::glad)
target_include_directories(imgui PUBLIC ${IMGUI_DIR} ${IMPLOT_DIR} ${IMGUI_DIR}/backends)
