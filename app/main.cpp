#include <exception>
#include <print>
#include "Application.hpp"

int main() {
    Core::app_specs specs;
    specs.name           = "LambdaPlotter";
    specs.w_specs.width  = 840;
    specs.w_specs.height = 640;

    try {
        Core::Application app(specs);
        app.run();
    } catch (const std::exception &e) {
        std::println(stderr, "%s", e.what());
    }

    return 0;
}
