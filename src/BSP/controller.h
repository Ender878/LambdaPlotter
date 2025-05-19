#ifndef __CONTROLLER_H__
#define __CONTROLLER_H__

#include "serial.h"
#include <atomic>
#include <deque>
#include <optional>
#include <vector>

namespace BSP {
    class Controller {
        private:
            // `combobox_port_index` and `combobox_baud_index` store the index of the selected option
            static std::optional<size_t> combobox_port_index;
            static size_t combobox_baud_index;

            // the following boolean flags are used to track the state of the
            // open/close button, the state of the serial reading operations and 
            // wether the list of available ports should be refreshed.
            static bool button_status;
            static std::atomic<bool> should_read;
            static std::atomic<bool> refresh_ports;
            static std::string current_port;

            // data structure containing the current plot data
            static std::deque<double> plot_data_list;

            static void startSerialReading(Serial& s);
            static void processData(std::vector<char> buffer);
        public:
            static void update(Serial& serial);
    };
}

#endif
