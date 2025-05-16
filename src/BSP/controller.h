#ifndef __CONTROLLER_H__
#define __CONTROLLER_H__

#include "serial.h"
#include <atomic>
#include <optional>

namespace BSP {
    extern const char* baud_rates_str[];

    class Controller {
        private:
            // boolean flags used to read the state of the view's components.
            // `combobox_port_index` and `combobox_baud_index` store the index of the selected option
            static std::optional<size_t> combobox_port_index;
            static size_t combobox_baud_index;

            // the following boolean flags are used to track the state of the
            // open/close button, the state of the serial reading operations and 
            // wether the list of available ports should be refreshed.
            static bool button_status;
            static std::atomic<bool> should_read;
            static bool refresh_ports;

            static void startSerialReading(const Serial& s);
        public:
            static void update(Serial& serial);
    };
}

#endif
