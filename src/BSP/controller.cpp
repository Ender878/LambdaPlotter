#include "controller.h"
#include "../common/defines.h"
#include "app.h"
#include "serial.h"
#include <algorithm>
#include <chrono>

#include <iterator>
#include <optional>
#include <print>
#include <thread>
#include <vector>
 
std::optional<size_t> BSP::Controller::combobox_port_index(std::nullopt);
size_t BSP::Controller::combobox_baud_index = 0;
bool BSP::Controller::button_status = false;
std::atomic<bool> BSP::Controller::should_read(false);
std::atomic<bool> BSP::Controller::refresh_ports(false);
std::string BSP::Controller::current_port("");

void BSP::Controller::update(Serial& serial) {
    std::vector<std::string>& serial_ports = serial.getSerialPorts(refresh_ports);

    if (refresh_ports) {
        
        // if the port list is empty, but the list index has a value, set the index to null
        if (serial_ports.empty() && combobox_port_index.has_value()) {
            combobox_port_index = std::nullopt;
        } else if (!serial_ports.empty() && combobox_port_index.has_value()) {
            // if the port list is not empty and the list index is set, check if the pointed value exists in the list and reassing the correct index
            // if the pointed element no longer exist, set the index to null
            if (std::any_of(serial_ports.begin(), serial_ports.end(), [](std::string v) { return v == current_port; })) {
                combobox_port_index = std::distance(serial_ports.begin(), std::find(serial_ports.begin(), serial_ports.end(), current_port)); 
            } else {
                combobox_port_index = std::nullopt;
            }
        }

    }

    // get current port
    current_port = combobox_port_index.has_value() ? serial_ports[combobox_port_index.value()] : "";

    BSP::Window::renderMenuBar(serial_ports, combobox_port_index, BSP::baud_rates, combobox_baud_index, button_status, should_read, refresh_ports);

    // check if the open/close port button has been pressed
    if (button_status) {
        // if we are not already reading and there is a port selected, start reading.
        // If we are reading and the button has been pressed, stop reading.
        if (combobox_port_index.has_value() && !should_read) {                        
            std::println("Opening port {} with baud rate {}", current_port, BSP::baud_rates[combobox_baud_index].str);

            serial.configurePort(combobox_port_index.value(), combobox_baud_index);

            startSerialReading(serial);
        } else if (should_read) {
            should_read = false;
        }
    }

}

void BSP::Controller::startSerialReading(Serial& s) {
    should_read = true;

    std::thread([&s]() {

        while (should_read) {
            std::vector<char> buf;

            if (s.read(buf)) {
                std::print("{}", buf.data());
            } else {
                // if the device get disconnected or we can't read, stop reading and refresh the port list
                s.close();
                should_read = false;
                refresh_ports = true;
            }
 
            std::this_thread::sleep_for(std::chrono::milliseconds(THREAD_READ_DELAY));
        }

    }).detach();
}
