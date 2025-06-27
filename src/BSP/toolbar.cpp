#include "toolbar.h"

#include <algorithm>
#include <string>

void BSP::ToolBar::updateSerialPorts(const std::vector<std::string> &serial_ports) {
    if (refresh_button) {
        // if the port list is empty, but the list index has a value, set the
        // index to null
        if (serial_ports.empty() && combobox_port_index.has_value()) {
            combobox_port_index = std::nullopt;
        } else if (!serial_ports.empty() && combobox_port_index.has_value()) {
            // if the port list is not empty and the list index is set, check if
            // the pointed value exists in the list and reassing the correct
            // index if the pointed element no longer exist, set the index to
            // null
            if (std::any_of(serial_ports.begin(), serial_ports.end(), [this](std::string v) { return v == current_port; })) {
                combobox_port_index = std::distance(serial_ports.begin(), std::find(serial_ports.begin(), serial_ports.end(), current_port));
            } else {
                combobox_port_index = std::nullopt;
            }
        }
    }

    current_port = combobox_port_index.has_value() ? serial_ports[combobox_port_index.value()] : "";
}

BSP::app_state_t BSP::ToolBar::getAppState(const app_state_t &curr_app_state) const {
    app_state_t state = curr_app_state;

    if (open_close_button) {
        // if we are not already reading and there is a port selected, start
        // reading. if we are reading and the button has been pressed, stop
        // reading.
        if (combobox_port_index.has_value() && curr_app_state == IDLE) {
            state = READING;
        } else if (curr_app_state == READING) {
            state = IDLE;
        }
    }

    return state;
}
