#include "window.h"
#include "../fonts/lucide.h"
#include "../fonts/roboto.h"
#include "../common/shared.h"
#include "../tinyfd/tinyfiledialogs.h"
#include "../bindings/imgui_impl_opengl3.h"
#include "../implot/implot.h"
#include "serial.h"
#include <GLFW/glfw3.h>
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <format>
#include <imgui.h>
#include <imgui_internal.h>
#include <mutex>
#include <stdexcept>
#include <string>
#include <termios.h>
#include <vector>

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

void BSP::Window::renderToolBar(ToolBar& tb, const std::vector<std::string>& serial_ports, app_state_t app_state, bool is_dataset_empty) {
    size_t baud_index = tb.getComboboxBaudIndex();
    size_t time_index = tb.getComboboxTimeIndex();
    std::optional<size_t> port_index = tb.getComboboxPortIndex();

    const char* port = port_index.has_value() && port_index <= serial_ports.size() ? serial_ports[port_index.value()].c_str() : "";
    const char* baud = BSP::baud_rates[baud_index].str;
    const char* time = BSP::time_windows[time_index].str;

    // buttons state variables
    bool refresh = false;
    bool open_close_bt = false;
    bool save = false;

    // setup toolbar frame
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImVec2(m_width * 0.2, m_height));

    ImGui::Begin("Toolbar", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);
 
    ImGui::SeparatorText("Serial interface");

    if (ImGui::BeginTable("##toolbar_layout", 2)) {
        ImGui::TableSetupColumn("Left", ImGuiTableColumnFlags_WidthStretch, 0.4f);
        ImGui::TableSetupColumn("Right", ImGuiTableColumnFlags_WidthStretch, 0.6f);

        ImGui::TableNextRow();
        ImGui::TableNextColumn();

        // ====== Serial ports combo box ====== 
        ImGui::Text("Serial port");
        ImGui::TableNextColumn();
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - 5);
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
        
        ImGui::TableNextRow();
        ImGui::TableNextColumn();

        // ====== Baud rates combo box ======
        ImGui::Text("Baud rate");
        ImGui::TableNextColumn();
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - 5);
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

        ImGui::EndTable();
    }

    // ====== Buttons ======
    if (ImGui::BeginTable("Buttons", 3)) {
        ImGui::TableSetupColumn("Left", ImGuiTableColumnFlags_WidthStretch, 0.3f);
        ImGui::TableSetupColumn("Center", ImGuiTableColumnFlags_WidthStretch, 0.3f);
        ImGui::TableSetupColumn("Right", ImGuiTableColumnFlags_WidthStretch, 0.3f);

        ImGui::TableNextRow();
        ImGui::TableNextColumn();

        // calc buttons size        
        ImVec2 buttons_size;
        buttons_size.x = buttons_size.y = ICON_SIZE + (ICON_SIZE / 6.0f) * 2;

        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (ImGui::GetContentRegionAvail().x - buttons_size.x) * 0.5);
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, ICON_SIZE);
        refresh = ImGui::Button(ICON_LC_REFRESH, buttons_size);

        ImGui::TableNextColumn();
        
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (ImGui::GetContentRegionAvail().x - buttons_size.x) * 0.5);
        open_close_bt = ImGui::Button(app_state == READING ? ICON_LC_PAUSE : ICON_LC_PLAY, buttons_size);
        
        ImGui::TableNextColumn();
        
        if (is_dataset_empty) {
            ImGui::BeginDisabled();
        }
    
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (ImGui::GetContentRegionAvail().x - buttons_size.x) * 0.5);
        save = ImGui::Button(ICON_LC_SAVE, buttons_size);
        ImGui::PopStyleVar();
        
        if (is_dataset_empty) {
            ImGui::EndDisabled();
        }

        ImGui::EndTable();
    }

    // stop reading if the Save button has been pressed
    if (app_state == READING && save) {
        open_close_bt = true;
    }

    renderSerialConf(app_state == IDLE);

    renderTelemetry();

    ImGui::SeparatorText("Plot configuration");

    // ====== Time window combo box ======
    ImGui::Text("Time Window");
    ImGui::SameLine();
    ImGui::PushItemWidth(m_width * 0.1);
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

    bool clear = ImGui::Button("Clear");

    tb.setComboboxPortIndex(port_index);
    tb.setComboboxBaudIndex(baud_index);
    tb.setComboboxTimeIndex(time_index);
    tb.setOpenCloseButton(open_close_bt);
    tb.setRefreshButton(refresh);
    tb.setSaveButton(save);
    tb.setClearButton(clear);

    //ImGui::ShowStyleEditor();
    //ImPlot::ShowStyleEditor();

    ImGui::End();
}

void BSP::Window::renderSerialConf(bool enabled) {

    if (ImGui::TreeNode("Advanced configuration")) {

        if (!enabled) {
            ImGui::BeginDisabled();
        }

        uint16_t parity      = Serial::getParity();
        uint16_t stop_bits   = Serial::getStopBits();
        uint16_t data_bits   = Serial::getDataBits();
        uint16_t flow_ctrl   = Serial::getFlowCtrl();

        // === Parity bit ===
        ImGui::Text("Parity:");

        if (ImGui::RadioButton("Disabled###ParityDIS", !parity)) {
            parity = 0;
        }

        if (ImGui::RadioButton("Even###ParityEV", parity == PARENB)) {
            parity = PARENB;
        }

        if (ImGui::RadioButton("Odd###ParityODD", parity == (PARENB | PARODD))) {
            parity = PARENB | PARODD;
        }

        // === Stop bits ===
        ImGui::Text("Stop bits:");

        if (ImGui::RadioButton("1###Stop1", stop_bits == 1)) {
            stop_bits = 1;
        }

        ImGui::SameLine();

        if (ImGui::RadioButton("2###Stop2", stop_bits == 2)) {
            stop_bits = 2;
        }

        // === Data bits ===
        ImGui::Text("Data bits:");

        if (ImGui::RadioButton("5###Data1", data_bits == CS5)) {
            data_bits = CS5;
        }

        ImGui::SameLine();

        if (ImGui::RadioButton("6###Data2", data_bits == CS6)) {
            data_bits = CS6;
        }

        if (ImGui::RadioButton("7###Data3", data_bits == CS7)) {
            data_bits = CS7;
        }

        ImGui::SameLine();

        if (ImGui::RadioButton("8###Data4", data_bits == CS8)) {
            data_bits = CS8;
        }

        // === Flow ctrl ===
        ImGui::Text("Flow control:");

        if (ImGui::RadioButton("Disabled###FlowDIS", !flow_ctrl)) {
            flow_ctrl = 0;
        }

        if (ImGui::RadioButton("Hardware###FlowHW", flow_ctrl == 1)) {
            flow_ctrl = 1;
        }

        if (ImGui::RadioButton("Software###FlowSW", flow_ctrl == 2)) {
            flow_ctrl = 2;
        }

        Serial::setParity(parity);
        Serial::setStopBits(stop_bits);
        Serial::setDataBits(data_bits);
        Serial::setFlowCtrl(flow_ctrl);

        if (!enabled) {
            ImGui::EndDisabled();
        }

        ImGui::TreePop();
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

void BSP::Window::renderPlot(const Telemetry& tel, const size_t time_window_index, const app_state_t app_state) {
    std::lock_guard<std::mutex> lock(BSP::plot_mtx);

    const std::vector<double>* times = tel.getTimestamps();
    const std::unordered_map<int, std::vector<double>>* data = tel.getData();

    ImGui::SetNextWindowPos(ImVec2(m_width * 0.2, 0));
    ImGui::SetNextWindowSize(ImVec2(m_width - m_width * 0.2, m_height));
    if (ImGui::Begin("##plot", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse)) {
        if (!data->empty()) {
            if (ImPlot::BeginPlot("##plot_win", ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y))) {
                    ImPlot::SetupAxis(ImAxis_X1, "Time");
                    ImPlot::SetupAxis(ImAxis_Y1, "##Data", (app_state == READING) ? ImPlotAxisFlags_AutoFit | ImPlotAxisFlags_RangeFit : 0);
    
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

                    // ImPlotRect limits = ImPlot::GetPlotLimits(ImAxis_X1);
                    // std::println("{} - {}", limits.Min().x, limits.Max().x);

                    // std::println("{}", int((last_time - first_time) * 1000));

                    for (const auto& stream : *data) {
                        std::string label = std::format("{}", stream.first);
    
                        ImPlot::PlotLine(label.c_str(), times->data(), stream.second.data(), stream.second.size());
                    }
    
                ImPlot::EndPlot();
            }
        } else {
            ImPlot::BeginPlot("##plot_win", ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y));
            ImPlot::EndPlot();
        }

        ImGui::End();
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
    const ImWchar icon_ranges[] = { 0xE132, 0xE151, 0 };

    ImFontConfig config;
    config.FontDataOwnedByAtlas = false;
    config.GlyphMinAdvanceX = ICON_SIZE;
    config.GlyphRanges = icon_ranges;
    config.GlyphOffset = ImVec2(0, 6.0f);
    config.MergeMode = true;

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
