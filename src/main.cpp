#include <LP/window.h>
#include <LP/controller.h>
#include <LP/shared.h>
#include <exception>
#include <iostream>
#include <ostream>

int main(void) {
    try {
        LP::Window::init(MIN_WIN_WIDTH, MIN_WIN_HEIGHT, "LambdaPlotter");

        while (LP::Window::render_mainloop([]() -> void {
            LP::Controller::update();
        }));

        LP::Controller::shutdown();
        LP::Window::destroy();
    } catch (const std::exception& e) {
        std::cerr << "ERROR: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
