#ifndef __SERIAL_H__
#define __SERIAL_H__

#include <cstddef>
#include <string>
#include <termios.h>
#include <vector>

namespace BSP {

    const int baud_rates[] = {B300, B600, B1200, B1800, B2400, B4800, B9600, B19200, B38400, B57600, B115200 };
    constexpr size_t baud_rates_size = sizeof(baud_rates) / sizeof(*baud_rates);

    class Serial {
        private:
            std::vector<std::string> m_serial_ports;

            int serial_port;

            std::vector<std::string> getAvailablePorts();
        public:
            Serial();
            ~Serial();

            bool configurePort(size_t t_port_index, size_t t_baud_index);

            bool readPort(std::vector<char>& buf) const;

            bool isPortConnected() const;

            std::vector<std::string>& getSerialPorts(bool refresh = false);
    };

    extern Serial serial;
}

#endif
