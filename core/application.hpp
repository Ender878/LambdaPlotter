#pragma once

#include <memory>
#include <window.hpp>
#include "imgui.h"

namespace Core {
    typedef struct app_specs {
        const char *title    = "";
        win_specs   w_specs  = win_specs();
    } app_specs;

    class Application {
        private:
            app_specs m_specs;
            std::shared_ptr<Window> m_window;
            bool m_running;

            ImGuiIO    im_io;
            ImGuiStyle im_style;
        public:
            explicit Application(app_specs m_specs = app_specs());
            ~Application();

            void run();
            void stop();

            std::shared_ptr<Window> get_window();
    };
}

