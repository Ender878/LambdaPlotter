#include <BSP/window.h>
#include "../fonts/lucide.h"
#include "../fonts/roboto.h"
#include <common/shared.h>
#include "../tinyfd/tinyfiledialogs.h"
#include "../bindings/imgui_impl_opengl3.h"
#include "../implot/implot.h"
#include <BSP/serial.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_internal.h>
#include <stdexcept>
#include <string>
#include <termios.h>

#include "../bindings/imgui_impl_glfw.h"

#define RGB(num) num / 255.0f

GLFWwindow* BSP::Window::window = nullptr;
ImGuiIO* BSP::Window::io = nullptr;
ImGuiStyle* BSP::Window::imgui_style = nullptr;
ImPlotStyle* BSP::Window::implot_style = nullptr;
int BSP::Window::m_width  = 0;
int BSP::Window::m_height = 0;

void BSP::Window::init(int t_width, int t_height, const char* title) {
    if (!glfwInit()) {
        throw std::runtime_error("Failed to initialize GLFW.");
    }

    m_width  = t_width;
    m_height = t_height;

    // set opengl version
    const char *glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    // create window
    window = glfwCreateWindow(m_width, m_height, title, nullptr, nullptr);

    if (!window) {
        throw std::runtime_error("Failed to create GLFW window");
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImPlot::CreateContext();

    io = &ImGui::GetIO();
    imgui_style = &ImGui::GetStyle();
    implot_style = &ImPlot::GetStyle();

    io->ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    loadDefaultFont();
    loadIconFont();
    
    setDarkStyle();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);
}

bool BSP::Window::renderMainWindow(std::function<void()> content) {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    glfwGetWindowSize(window, &m_width, &m_height);
    glfwPollEvents();
    ImGui::NewFrame();

    content();

    ImGui::EndFrame();
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glfwSwapBuffers(window);
    glClear(GL_COLOR_BUFFER_BIT);
    glfwPollEvents();

    return !glfwWindowShouldClose(window);
}

void BSP::Window::renderMenuBar(std::function<void()> content) {
    ImGui::SetNextWindowPos(ImVec2(0, 0)); 
    ImGui::SetNextWindowSize(ImVec2(m_width * 0.25, m_height));

    if (ImGui::Begin("MenuBar", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar)) {

        content();

        ImGui::End();
    }
}

void BSP::Window::destroy() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    ImPlot::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();
}

std::string BSP::Window::saveFileDialog(const char* default_path) {
    const char* path = tinyfd_saveFileDialog(
        "Save data", 
        default_path, 
        0, 
        nullptr, 
        "CSV Files (*.csv)"
    );

    if (path) {
        return path;
    }

    return std::string();
}

ImVec2 BSP::Window::getWindowSize() {
    return ImVec2(m_width, m_height);
}

void BSP::Window::renderTelemetry() {
    ImGui::SeparatorText("Telemetry");

    ImGui::Text("No telemetry received yet!");
}

void BSP::Window::loadDefaultFont() {
    ImFontConfig config;
    config.FontDataOwnedByAtlas = false;

    io->Fonts->AddFontFromMemoryTTF(roboto_ttf, roboto_ttf_len, 18.0f, &config); 
}

void BSP::Window::loadIconFont() {
    const ImWchar icon_ranges[] = { 0xE132, 0xE158, 0 };

    ImFontConfig config;
    config.FontDataOwnedByAtlas = false;
    config.GlyphMinAdvanceX     = ICON_SIZE;
    config.GlyphRanges          = icon_ranges;
    config.GlyphOffset          = ImVec2(0, 6.0f);
    config.MergeMode            = true;

    io->Fonts->AddFontFromMemoryTTF(lucide_ttf, lucide_ttf_len, ICON_SIZE, &config);
}

void BSP::Window::setDarkStyle() {
    ImGui::StyleColorsDark();

    // widget spacing
    imgui_style->ItemSpacing                        = ImVec2(12, 8); 
    imgui_style->ItemInnerSpacing                   = ImVec2(2, 3);
    
    // widget style 
    imgui_style->CellPadding                        = ImVec2(5, 5);
    imgui_style->FramePadding                       = ImVec2(3, 3);
    imgui_style->FrameRounding                      = 4.0f;
    imgui_style->FrameBorderSize                    = 1.0f;

    // Colors 
    imgui_style->Colors[ImGuiCol_WindowBg]          = ImVec4(0.09f, 0.1f, 0.15f, 1.0f);
    imgui_style->Colors[ImGuiCol_FrameBg]           = ImVec4(0.17f, 0.2f, 0.22f, 1.0f);
    imgui_style->Colors[ImGuiCol_FrameBgHovered]    = ImVec4(0.17f, 0.2f, 0.22f, 1.0f);
    imgui_style->Colors[ImGuiCol_FrameBgActive]     = ImVec4(0.17f, 0.2f, 0.42f, 1.0f);
    imgui_style->Colors[ImGuiCol_HeaderHovered]     = ImVec4(0.17f, 0.2f, 0.22f, 1.0f);
    imgui_style->Colors[ImGuiCol_HeaderActive]      = ImVec4(0.17f, 0.2f, 0.22f, 1.0f);

    imgui_style->Colors[ImGuiCol_Button]            = ImVec4(RGB(13), RGB(113), RGB(235), 1.0f);
    imgui_style->Colors[ImGuiCol_ButtonHovered]     = ImVec4(0.0f, 0.6f, 0.2f, 1.0f);
    imgui_style->Colors[ImGuiCol_ButtonActive]      = ImVec4(0.0f, 0.5f, 0.2f, 1.0f);

    imgui_style->Colors[ImGuiCol_CheckMark]         = ImVec4(0.0f, 0.6f, 0.2f, 1.0f);

    // implot style
    implot_style->LineWeight                        = 2.0f;
    implot_style->UseLocalTime                      = true;

    implot_style->Colors[ImPlotCol_AxisBgHovered]   = ImVec4(0.17f, 0.2f, 0.22f, 1.0f);
    implot_style->Colors[ImPlotCol_AxisBgActive]    = ImVec4(0.17f, 0.2f, 0.22f, 1.0f);
}
