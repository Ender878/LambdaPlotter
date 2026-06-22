#include "Application.hpp"
#include <GLFW/glfw3.h>
#include <glm/common.hpp>
#include <memory>
#include <stdexcept>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

namespace Core {

    Application::Application(const app_specs &specs)
     : m_specs(specs), m_running(false) 
    {
        if (!glfwInit()) {
            throw std::runtime_error("[ERROR]: Could not initialize GLFW");
        }

        if (this->m_specs.w_specs.title.empty())
            this->m_specs.w_specs.title = this->m_specs.name;

        m_window = std::make_shared<Window>(this->m_specs.w_specs);

        m_window->create();
        
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        im_io    = ImGui::GetIO();
        im_style = ImGui::GetStyle();

        ImGui_ImplGlfw_InitForOpenGL(m_window->get_handle(), true);
        ImGui_ImplOpenGL3_Init();
    }

    void Application::run() {
        m_running = true;

        float last_time = get_time();
        
        while (m_running) {
            if (m_window->should_close()) {
                stop();
                break;
            }

            float current_time = get_time();
            float delta_time   = glm::clamp(current_time - last_time, 0.001f, 0.1f);
            last_time = current_time;
            
            for (const auto& layer : m_layers)
                layer->on_update(delta_time);

            for (const auto& layer : m_layers)
                layer->on_render();

            m_window->update();
        }
    }
    
    void Application::stop() {
        m_running = false;
    }

    Application::~Application() {
        m_window->destroy();
        glfwTerminate();
    }

    std::shared_ptr<Window> Application::get_window() {
        return m_window;
    }

    float Application::get_time() {
        return static_cast<float>(glfwGetTime());
    }
}
