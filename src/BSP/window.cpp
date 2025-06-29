#include "window.h"
#include "../common/shared.h"
#include "../tinyfd/tinyfiledialogs.h"
#include "../bindings/imgui_impl_opengl3.h"
#include "../implot/implot.h"
#include <GLFW/glfw3.h>
#include <algorithm>
#include <cstddef>
#include <format>
#include <imgui.h>
#include <mutex>
#include <stdexcept>
#include <string>
#include <vector>

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

    ImGui::Begin("better_serial_plotter", nullptr, BSP::MAIN_WIN_FLAGS);

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

void BSP::Window::renderToolBar(ToolBar& tb, const std::vector<std::string>& serial_ports, app_state_t app_state, bool is_dataset_empty) {
    size_t baud_index = tb.getComboboxBaudIndex();
    size_t time_index = tb.getComboboxTimeIndex();
    std::optional<size_t> port_index = tb.getComboboxPortIndex();

    const char* port = port_index.has_value() && port_index <= serial_ports.size() ? serial_ports[port_index.value()].c_str() : "";
    const char* baud = BSP::baud_rates[baud_index].str;
    const char* time = BSP::time_windows[time_index].str;

    ImGui::PushItemWidth(m_width / 5.0);

    // ====== Serial ports combo box ====== 
    ImGui::Text("Serial");
    ImGui::SameLine();
    if (ImGui::BeginCombo("##serial", port)) {
        for (size_t i = 0; i < serial_ports.size(); i++) {
            bool is_selected = port_index == i;

            if (ImGui::Selectable(serial_ports[i].c_str(), is_selected)) {
                port_index = i;
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
            bool selected = baud_index == i;

            if (ImGui::Selectable(BSP::baud_rates[i].str, selected)) {
                baud_index = i;
                ImGui::SetItemDefaultFocus();
            }

        }
        ImGui::EndCombo();
    }

    ImGui::SameLine();

    // ====== Time window combo box ======
    ImGui::Text("Time Window");
    ImGui::SameLine();
    if (ImGui::BeginCombo("##time", time)) {
        for (size_t i = 0; i < BSP::baud_time_size; i++) {
            bool selected = time_index == i;

            if (ImGui::Selectable(BSP::time_windows[i].str, selected)) {
                time_index = i;
                ImGui::SetItemDefaultFocus();
            }

        }
        ImGui::EndCombo();
    }

    ImGui::PopItemWidth();

    ImGui::SameLine();

    bool open_close_bt = ImGui::Button(app_state == READING ? "Close" : "Open");

    ImGui::SameLine();

    bool refresh = ImGui::Button("Refresh");

    ImGui::SameLine();

    if (is_dataset_empty) {
        ImGui::BeginDisabled();
    }

    bool save = ImGui::Button("Save");

    if (is_dataset_empty) {
        ImGui::EndDisabled();
    }

    // stop reading if the Save button has been pressed
    if (app_state == READING && save) {
        open_close_bt = true;
    }

    ImGui::SameLine();

    bool clear = ImGui::Button("Clear");

    tb.setComboboxPortIndex(port_index);
    tb.setComboboxBaudIndex(baud_index);
    tb.setComboboxTimeIndex(time_index);
    tb.setOpenCloseButton(open_close_bt);
    tb.setRefreshButton(refresh);
    tb.setSaveButton(save);
    tb.setClearButton(clear);
}

void BSP::Window::destroy() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    ImPlot::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();
}

void BSP::Window::renderPlot(const Telemetry& tel, const size_t time_window_index, const app_state_t app_state) {
    std::lock_guard<std::mutex> lock(BSP::plot_mtx);

    const std::vector<double>* times = tel.getTimestamps();
    const std::unordered_map<int, std::vector<double>>* data = tel.getData();
    
    if (!data->empty()) {

        if (ImPlot::BeginPlot("##plot_win")) {
                ImPlot::SetupAxis(ImAxis_X1, "Time (s)");
                ImPlot::SetupAxis(ImAxis_Y1, "Data", (app_state == READING) ? ImPlotAxisFlags_AutoFit | ImPlotAxisFlags_RangeFit : 0);

                ImPlot::SetupAxisScale(ImAxis_X1, ImPlotScale_Time);

                double first_time = times->front();
                double last_time  = times->back();

                int time_window = BSP::time_windows[time_window_index].value;
                
                // Set time window, if there is.
                double window_start = (time_window != 0)
                    ? std::max(first_time, last_time - time_window)
                    : first_time;

                if (app_state == READING)
                    ImPlot::SetupAxisLimits(ImAxis_X1, window_start, last_time, ImGuiCond_Always);

                for (const auto& stream : *data) {
                    std::string label = std::format("{}", stream.first);

                    ImPlot::PlotLine(label.c_str(), times->data(), stream.second.data(), stream.second.size());
                }

            ImPlot::EndPlot();
        }
    } else {
        ImPlot::BeginPlot("##plot_win");
        ImPlot::EndPlot();
    }
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
