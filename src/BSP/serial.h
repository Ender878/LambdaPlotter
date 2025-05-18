#ifndef __SERIAL_H__
#define __SERIAL_H__

#include <cstddef>
#include <string>
#include <termios.h>
#include <vector>

namespace BSP {
    typedef struct baud_rate_t {
        const char* str;
        const int   value;
    } baud_rate_t;

    extern const baud_rate_t baud_rates[];
    extern const size_t baud_rates_size;

    class Serial {
        private:
            std::vector<std::string> m_serial_ports;

            int serial_port;

            std::vector<std::string> getAvailablePorts();
        public:
            Serial();
            ~Serial();

            bool configurePort(size_t t_port_index, size_t t_baud_index);

            bool read(std::vector<char>& buf) const;

            bool isPortConnected() const;

            void close();

            std::vector<std::string>& getSerialPorts(bool refresh = false);
    };

    extern Serial serial;
}

#endif
