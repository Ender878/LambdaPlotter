#ifndef __SERIAL_H__
#define __SERIAL_H__

#include <cstddef>
#include <string>
#include <termios.h>
#include <vector>

#define DEFAULT_BUF_SIZE 1024

namespace BSP {
    class Serial {
        private:
            int serial_port_fd;
            
            static std::string last_open_port;
            static std::vector<std::string> serial_ports;

            static std::vector<std::string> readSystemPorts();
        public:
            Serial(const char* port, size_t baud);
            ~Serial();

            bool read(std::vector<char>& buf) const;

            bool isPortConnected() const;

            int getFileDescriptor() const;

            void close();

            static std::vector<std::string>& getSerialPorts(bool refresh = false);
            static std::string getLastOpenPort();
            static void setLastOpenPort(const char* port);
    };
}

#endif
