#ifndef __SERIAL_H__
#define __SERIAL_H__

#include <cstddef>
#include <cstdint>
#include <string>
#include <termios.h>
#include <vector>

#define DEFAULT_BUF_SIZE 1024

namespace BSP {
    class Serial {
        private:
            int serial_port_fd;
            
            static uint16_t parity;
            static uint16_t stop_bits;
            static uint16_t data_bits;
            static uint16_t flow_ctrl;

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

            static inline void setParity(uint16_t value)   { parity = value; }
            static inline void setStopBits(uint16_t value) { stop_bits = value; }
            static inline void setDataBits(uint16_t value) { data_bits = value; }
            static inline void setFlowCtrl(uint16_t value) { flow_ctrl = value; }

            static inline uint16_t getParity()   { return parity; }
            static inline uint16_t getStopBits() { return stop_bits; }
            static inline uint16_t getDataBits() { return data_bits; }
            static inline uint16_t getFlowCtrl() { return flow_ctrl; }
    };
}

#endif
