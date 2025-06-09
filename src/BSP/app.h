#ifndef __APP_H__
#define __APP_H__

#include "../bindings/imgui_impl_glfw.h"
#include "serial.h"
#include "../implot/implot.h"
#include <atomic>
#include <deque>
#include <functional>
#include <imgui.h>
#include <optional>
#include <string>
#include <vector>

namespace BSP {
    constexpr ImGuiWindowFlags MAIN_WIN_FLAGS = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse;

    class Window {
        private: 
            static GLFWwindow*  window;
            static ImGuiIO*     io;
            static ImGuiStyle*  imgui_style;
            static ImPlotStyle* implot_style;

            static int m_width;
            static int m_height;
        public:
            static void init(int t_width, int t_height, const char* title);

            static bool renderMainWindow(std::function<void()> content);

            static void renderMenuBar(const std::vector<std::string>& serial_ports, std::optional<size_t>& current_port, const baud_rate_t* b_rates, size_t& current_rate, bool& open, bool should_read, std::atomic<bool>& refresh_ports);

            static void renderPlot(const std::deque<double>& data, const std::deque<long>& timestamp);

            static void destroy();

            static ImVec2 getWindowSize();
    };
}

#endif
