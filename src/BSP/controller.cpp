#include "controller.h"
#include "app.h"
#include "serial.h"
#include <cstddef>

#include <optional>
#include <print>
#include <vector>

std::optional<size_t> BSP::Controller::serial_port_index = std::nullopt;
size_t BSP::Controller::serial_baud_index = 0;
bool BSP::Controller::button_status = false;
bool BSP::Controller::should_read = false;

void BSP::Controller::update(Serial& serial) {
    static bool refresh_ports = false;

    if (refresh_ports && !should_read) {
        serial_port_index = std::nullopt;
    }

    BSP::Window::renderMenuBar(serial.getSerialPorts(refresh_ports), serial_port_index, BSP::baud_rates_str, serial_baud_index, button_status, should_read, refresh_ports);

    // check if the open/close port button has been pressed
    if (button_status) {
        // if we are not already reading and there is a port selected, start reading.
        // If we are reading and the button has been pressed, stop reading.
        if (serial_port_index.has_value() && !should_read) {            
            auto port = serial.getSerialPorts()[serial_port_index.value()];
            
            std::println("Opening port {} with baud rate {}", port, BSP::baud_rates_str[serial_baud_index]);

            serial.configurePort(serial_port_index.value(), serial_baud_index);

            should_read = true;
        } else if (should_read) {
            should_read = false;
        }
    }

    if (should_read) {
        std::vector<char> buf;
        
        if (serial.readPort(buf)) {
            std::println("{}", buf.data());
        } else {
            should_read = false;
            serial_port_index = std::nullopt;
            refresh_ports = true;
        }
    }

}

namespace BSP {
    const char* baud_rates_str[] = { "300", "600", "1200", "1800", "2400", "4800", "9600", "19200", "38400", "57600", "115200" };
}
