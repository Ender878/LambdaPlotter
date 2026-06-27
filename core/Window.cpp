#include "Window.hpp"
#include <GLFW/glfw3.h>
#include <glad/gl.h>
#include <stdexcept>

namespace Core {
    Window::Window(win_specs specs) : m_specs(std::move(specs)), m_handle(nullptr) {}

    Window::~Window() {
        destroy();
    }

    void Window::create() {

    #if defined(IMGUI_IMPL_OPENGL_ES2)
        // GL ES 2.0 + GLSL 100
        // const char* glsl_version = "#version 100";
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
        glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
    #elif defined(__APPLE__)
        // GL 3.2 + GLSL 150
        // const char* glsl_version = "#version 150";
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // 3.2+ only
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);           // Required on Mac
    #else
        // GL 3.0 + GLSL 130
        // const char* glsl_version = "#version 130";
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    #endif

        m_handle = glfwCreateWindow(m_specs.width, m_specs.height, m_specs.title.c_str(), nullptr, nullptr);
        if (!m_handle) {
            glfwTerminate();
            throw std::runtime_error("[ERROR]: Could not create GLFW window.");
        }

        glfwMakeContextCurrent(m_handle);
        gladLoadGL(glfwGetProcAddress);
        glfwSwapInterval(1);
    }

    void Window::update() {
        glfwSwapBuffers(m_handle);
        glfwPollEvents();
    }

    void Window::destroy() {
        if (m_handle) {
            glfwDestroyWindow(m_handle);
        }
        
        m_handle = nullptr;
    }

    glm::vec2 Window::get_frame_buf_size() const {
        int w, h;
        glfwGetFramebufferSize(m_handle, &w, &h);
        return {w, h};
    }

    bool Window::should_close() const {
        return glfwWindowShouldClose(m_handle);
    }

    GLFWwindow *Window::get_handle() const {
        return m_handle;
    }
}
