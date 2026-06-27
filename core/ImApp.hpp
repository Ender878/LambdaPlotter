#pragma once

#include <memory>
#include <Window.hpp>
#include <string>
#include <type_traits>
#include <vector>
#include "ILayer.hpp"
#include "imgui.h"  

namespace Core {
    typedef struct app_specs {
        std::string      name         = "ImApp";
        win_specs        w_specs      = win_specs();
        ImGuiConfigFlags config_flags = 0;
    } app_specs;

    class ImApp {
        private:
            app_specs m_specs;
            std::shared_ptr<Window> m_window;
            std::vector<std::unique_ptr<ILayer>> m_layers;
            bool m_running;

            ImGuiIO*    im_io;
            ImGuiStyle* im_style;
        public:
            explicit ImApp(app_specs specs = app_specs());
            ~ImApp();

            void run();
            void stop();

            std::shared_ptr<Window> get_window();
            
            template <typename Layer>
            requires std::is_base_of_v<ILayer, Layer>
            void push_layer() {
                m_layers.push_back(std::make_unique<Layer>());
            }

            ImGuiIO*    get_im_io();
            ImGuiStyle* get_im_style();

            [[nodiscard]] static float get_time();
    };
}
