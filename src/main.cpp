#include "BSP/window.h"
#include "BSP/controller.h"
#include "common/shared.h"
#include <exception>
#include <print>

int main(void) {

    try {
        BSP::Window::init(WIN_WIDTH, WIN_HEIGHT, "Better Serial Plotter");

        while (BSP::Window::renderMainWindow([]() -> void {
            BSP::Controller::update();
        }));

        BSP::Controller::shutdown();

        BSP::Window::destroy();
    } catch (const std::exception& e) {
        std::println(stderr, "ERROR: {}", e.what());
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
