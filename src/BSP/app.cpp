#include "app.h"
#include "../bindings/imgui_impl_opengl3.h"
#include "../implot/implot.h"
#include "serial.h"
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <print>
#include <stdexcept>
#include <vector>

GLFWwindow* BSP::Window::window = nullptr;
ImGuiIO* BSP::Window::io = nullptr;
ImGuiStyle* BSP::Window::style = nullptr;
int BSP::Window::m_width = 0;
int BSP::Window::m_height = 0;

void BSP::Window::init(int t_width, int t_height, const char* title) {
    if (!glfwInit()) {
        throw std::runtime_error("Failed to initialize GLFW.");
    }

    m_width = t_width;
    m_height = t_height;

    // determine opengl version
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
    style = &ImGui::GetStyle();

    io->ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);
}

bool BSP::Window::renderMainWindow(std::function<void()> content) {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    glfwGetWindowSize(window, &m_width, &m_height);
    glfwPollEvents();

    ImGui::NewFrame();
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(BSP::Window::getWindowSize());

    ImGui::Begin("better_serial_monitor", nullptr, BSP::MAIN_WIN_FLAGS);

    content();

    ImGui::End();

    ImGui::EndFrame();
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glfwSwapBuffers(window);
    glClear(GL_COLOR_BUFFER_BIT);
    glfwPollEvents();

    return !glfwWindowShouldClose(window);
}

void BSP::Window::renderMenuBar(const std::vector<std::string>& serial_ports, std::optional<size_t>& current_port, const baud_rate_t* b_rates, size_t& current_rate, bool& open, bool should_read, std::atomic<bool>& refresh_ports) {
    const char* port = current_port.has_value() && current_port <= serial_ports.size() ? serial_ports[current_port.value()].c_str() : "";
    const char* baud = b_rates[current_rate].str;

    ImGui::PushItemWidth(m_width / 3.0);

    // ====== Serial ports combo box ====== 
    ImGui::Text("Serial");
    ImGui::SameLine();
    if (ImGui::BeginCombo("##serial", port)) {
        for (size_t i = 0; i < serial_ports.size(); i++) {
            bool is_selected = current_port == i;

            if (ImGui::Selectable(serial_ports[i].c_str(), is_selected)) {
                current_port = i;
                ImGui::SetItemDefaultFocus();
            }

        }
        ImGui::EndCombo();
    }

    ImGui::SameLine();

    // ====== Baud rates combo box ======
    ImGui::Text("Baud rate");
    ImGui::SameLine();
    if (ImGui::BeginCombo("##baud", baud)) {
        for (size_t i = 0; i < BSP::baud_rates_size; i++) {
            bool selected = current_rate == i;

            if (ImGui::Selectable(b_rates[i].str, selected)) {
                current_rate = i;
                ImGui::SetItemDefaultFocus();
            }

        }
        ImGui::EndCombo();
    }

    ImGui::PopItemWidth();

    ImGui::SameLine();

    open = ImGui::Button(should_read ? "Close" : "Open");

    ImGui::SameLine();

    refresh_ports = ImGui::Button("Refresh");
} 

void BSP::Window::destroy() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    ImPlot::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();
}

void BSP::Window::renderPlot(const std::deque<double>& data) {
    if (ImPlot::BeginPlot("plot")) {
        if (data.empty()) {
            ImPlot::EndPlot();

            return;
        }

        std::vector<double> data_vec(data.begin(), data.end());

        ImPlot::PlotLine("##plot", data_vec.data(), data.size());

        ImPlot::EndPlot();
    }
}

ImVec2 BSP::Window::getWindowSize() {
    return ImVec2(m_width, m_height);
}
