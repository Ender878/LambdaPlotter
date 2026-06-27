#include "ImApp.hpp"
#include <GLFW/glfw3.h>
#include <glad/gl.h>
#include <glm/common.hpp>
#include <memory>
#include <stdexcept>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

namespace Core {
    ImApp::ImApp(app_specs specs)
     : m_specs(std::move(specs)), m_running(false) 
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
        im_io    = &ImGui::GetIO(); (void)im_io;
        im_style = &ImGui::GetStyle();

        im_io->ConfigFlags = this->m_specs.config_flags;

        ImGui_ImplGlfw_InitForOpenGL(m_window->get_handle(), true);
        ImGui_ImplOpenGL3_Init();
    }

    void ImApp::run() {
        m_running = true;

        float last_time = get_time();

        while (m_running) {
            auto frame_buf_size = m_window->get_frame_buf_size();
            glViewport(0, 0, frame_buf_size.x, frame_buf_size.y);
            glClear(GL_COLOR_BUFFER_BIT);

            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();
            ImGui::DockSpaceOverViewport();

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

            if (im_io->ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
                GLFWwindow* backup_current_context = glfwGetCurrentContext();
                ImGui::UpdatePlatformWindows();
                ImGui::RenderPlatformWindowsDefault();
                glfwMakeContextCurrent(backup_current_context);
            }

            ImGui::Render();
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

            m_window->update();
        }
    }
    
    void ImApp::stop() {
        m_running = false;
    }

    ImApp::~ImApp() {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();

        m_window->destroy();
        glfwTerminate();
    }

    std::shared_ptr<Window> ImApp::get_window() {
        return m_window;
    }

    ImGuiIO* ImApp::get_im_io() {
        return im_io;
    }

    ImGuiStyle* ImApp::get_im_style() {
        return im_style;
    }

    float ImApp::get_time() {
        return static_cast<float>(glfwGetTime());
    }
}
