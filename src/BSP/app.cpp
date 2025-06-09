#include "app.h"
#include "../common/defines.h"
#include "../bindings/imgui_impl_opengl3.h"
#include "../implot/implot.h"
#include "serial.h"
#include <GLFW/glfw3.h>
#include <algorithm>
#include <deque>
#include <imgui.h>
#include <mutex>
#include <print>
#include <stdexcept>
#include <vector>

GLFWwindow* BSP::Window::window = nullptr;
ImGuiIO* BSP::Window::io = nullptr;
ImGuiStyle* BSP::Window::imgui_style = nullptr;
ImPlotStyle* BSP::Window::implot_style = nullptr;
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
    imgui_style = &ImGui::GetStyle();
    implot_style = &ImPlot::GetStyle();

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

    refresh_ports.store(ImGui::Button("Refresh"));
} 

void BSP::Window::destroy() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    ImPlot::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();
}

void BSP::Window::renderPlot(const std::deque<double>& data, const std::deque<long>& timestamps) {
    if (!data.empty()) {
        std::deque<double> local_data;
        std::deque<long> local_timestamps;

        {
            std::lock_guard<std::mutex> lock(mtx);

            local_data = data;
            local_timestamps = timestamps;
        }

        double data_max = *std::max_element(local_data.begin(), local_data.end());
        double data_min = *std::min_element(local_data.begin(), local_data.end());

        ImPlot::SetNextAxesLimits(local_timestamps.front(), local_timestamps.back(), data_min - 10, data_max + 10, ImPlotCond_Always);

        if (ImPlot::BeginPlot("##plot_win")) {
                ImPlot::SetupAxes("Time", "Data");

                ImPlot::SetupAxisFormat(ImAxis_X1, "%.0f ms");

                std::vector<double> data_vec(local_data.begin(), local_data.end());
                std::vector<double> timestamps_vec(local_timestamps.begin(), local_timestamps.end());
    
                ImPlot::PlotLine("##plot", timestamps_vec.data(), data_vec.data(), local_timestamps.size());
    
            ImPlot::EndPlot();
        }
    } else {
        ImPlot::BeginPlot("##plot_win");
        ImPlot::EndPlot();
    }

}

ImVec2 BSP::Window::getWindowSize() {
    return ImVec2(m_width, m_height);
}
