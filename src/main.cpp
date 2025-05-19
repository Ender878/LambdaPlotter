#include "BSP/app.h"
#include "BSP/controller.h"
#include "common/defines.h"
#include "BSP/serial.h"
#include <exception>
#include <print>

int main(int, char **) {

    try {
        BSP::Window::init(WIN_WIDTH, WIN_HEIGHT, "Better Serial Plotter");

        while (BSP::Window::renderMainWindow([]() -> void {
            BSP::Controller::update(BSP::serial);
        }));

        BSP::Window::destroy();
    } catch (const std::exception& e) {
        std::println(stderr, "ERROR: {}", e.what());
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
