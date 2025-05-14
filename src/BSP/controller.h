#ifndef __CONTROLLER_H__
#define __CONTROLLER_H__

#include "serial.h"
#include <optional>

namespace BSP {
    extern const char* baud_rates_str[];

    class Controller {
        private:
            static std::optional<size_t> serial_port_index;
            static size_t serial_baud_index;
            static bool button_status;
            static bool should_read;

        public:
            static void update(Serial& serial);
    };
}

#endif
