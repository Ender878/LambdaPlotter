#include "application.hpp"
#include <GLFW/glfw3.h>
#include <memory>
#include <stdexcept>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

namespace Core {

    Application::Application(app_specs m_specs)
     : m_specs(m_specs), m_running(false) 
    {
        if (!glfwInit()) {
            throw std::runtime_error("[ERROR]: Could not initialize GLFW");
        }

        m_window = std::make_shared<Window>(m_specs.w_specs);

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

        while (m_running) {
            

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
}
