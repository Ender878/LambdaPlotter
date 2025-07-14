#ifndef __APP_H__
#define __APP_H__

#include "../bindings/imgui_impl_glfw.h"
#include "../implot/implot.h"
#include "telemetry.h"
#include "toolbar.h"
#include <functional>
#include <imgui.h>
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

            static void setDarkStyle();
            static void loadDefaultFont();
            static void loadIconFont();
        public:
            static void init(int t_width, int t_height, const char* title);

            static bool renderMainWindow(std::function<void()> content);

            static void renderToolBar(ToolBar& tb, const std::vector<std::string>& serial_ports, app_state_t app_state, bool is_dataset_empty);

            // TODO
            static void renderSerialConf(bool enabled);

            static void renderTelemetry();

            static void renderPlotConf(); 

            static void renderPlot(const Telemetry& tel, const size_t time_window_index, const app_state_t app_state);

            static std::string saveFileDialog(const char* default_path);

            static void destroy();

            static ImVec2 getWindowSize();
    };
}

#endif
