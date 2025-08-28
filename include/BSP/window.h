#ifndef __APP_H__
#define __APP_H__

#include <BSP/telemetry.h>
#include <BSP/toolbar.h>
#include <functional>
#include <imgui.h>
#include <string>

struct GLFWwindow;
struct ImGuiIO;
struct ImGuiStyle;
struct ImPlotStyle;

namespace BSP {
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
            /**
             * @brief Initialize the GLFW window
             * 
             * @param t_width  initial window width
             * @param t_height initial window height
             * @param title    window title
             */
            static void init(int t_width, int t_height, const char* title);

            /**
             * @brief Render the application's main loop
             * 
             * @param content lamba containing the window's content
             * @return true   if the app should run
             * @return false  if the app should close
             */
            static bool render_mainloop(std::function<void()> content);

            /**
             * @brief Render the application's toolbar
             * 
             * @param content lamba containing the toolbar content
             */
            static void render_toolbar(std::function<void()> content);

            /**
             * @brief Render the Save file dialog
             * 
             * @param default_path default save path
             * @return select save path
             */
            static std::string render_save_fd(const char* default_path);

            /**
             * @brief GLFW window close calls
             * 
             */
            static void destroy();

            static ImVec2 getWindowSize();
    };
}

#endif
