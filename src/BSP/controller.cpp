#include "controller.h"
#include "../common/defines.h"
#include "app.h"
#include "serial.h"
#include <chrono>
#include <cstddef>

#include <optional>
#include <print>
#include <thread>
#include <vector>

std::optional<size_t> BSP::Controller::combobox_port_index(std::nullopt);
size_t BSP::Controller::combobox_baud_index = 0;
bool BSP::Controller::button_status = false;
std::atomic<bool> BSP::Controller::should_read(false);
bool BSP::Controller::refresh_ports = false;

// TODO: implement thread for reading from the serial port
void BSP::Controller::update(Serial& serial) {
    std::vector<std::string>& serial_ports = serial.getSerialPorts(refresh_ports);

    // when refreshing, reset ports' index if it is greater than the new length of the port list.
    if (refresh_ports && (combobox_port_index.has_value() && combobox_port_index.value() >= serial_ports.size())) {
        combobox_port_index = std::nullopt;
    }

    BSP::Window::renderMenuBar(serial_ports, combobox_port_index, BSP::baud_rates_str, combobox_baud_index, button_status, should_read, refresh_ports);

    // check if the open/close port button has been pressed
    if (button_status) {
        // if we are not already reading and there is a port selected, start reading.
        // If we are reading and the button has been pressed, stop reading.
        if (combobox_port_index.has_value() && !should_read) {            
            auto port = serial_ports[combobox_port_index.value()];
            
            std::println("Opening port {} with baud rate {}", port, BSP::baud_rates_str[combobox_baud_index]);

            serial.configurePort(combobox_port_index.value(), combobox_baud_index);

            startSerialReading(serial);
        } else if (should_read) {
            should_read = false;
        }
    }

}

void BSP::Controller::startSerialReading(const Serial& s) {
    should_read = true;

    std::thread([&s]() {

        while (should_read) {
            std::vector<char> buf;

            if (s.readPort(buf)) {
                std::print("{}", buf.data());
            } else {
                should_read = false;
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(THREAD_READ_DELAY));
        }

    }).detach();
}

namespace BSP {
    const char* baud_rates_str[] = { "300", "600", "1200", "1800", "2400", "4800", "9600", "19200", "38400", "57600", "115200" };
}
