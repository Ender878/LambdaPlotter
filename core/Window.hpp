#pragma once

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <cstdint>
#include <string>

namespace Core {
    typedef struct win_specs {
        std::string title;
        uint16_t    width  = 0;
        uint16_t    height = 0;
    } win_specs;

    class Window {
        private:
            win_specs   m_specs;
            GLFWwindow* m_handle;
        public:
            explicit Window(const win_specs& specs = win_specs());
            ~Window();

            void create();
            void update();
            void destroy();

            [[nodiscard]] glm::vec2 get_frame_buf_size() const;

            [[nodiscard]] bool should_close() const;
            [[nodiscard]] GLFWwindow *get_handle() const;
    };
}
