#pragma once

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <cstdint>

namespace Core {
    typedef struct win_specs {
        const char* title;
        uint16_t    width;
        uint16_t    height;
    } win_specs;

    class Window {
        private:
            win_specs   m_specs;
            GLFWwindow* m_handle;
        public:
            explicit Window(win_specs specs = win_specs()) : m_handle(nullptr), m_specs(specs) {}
            ~Window();

            void create();
            void update();
            void destroy();

            [[nodiscard]] glm::vec2 get_frame_buf_size() const;

            [[nodiscard]] bool should_close() const;
            [[nodiscard]] GLFWwindow *get_handle() const;
    };
}
