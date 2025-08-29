#ifndef __SERIAL_H__
#define __SERIAL_H__

#include <cstddef>
#include <cstdint>
#include <string>
#include <termios.h>
#include <vector>

#define DEFAULT_BUF_SIZE 1024
#define FLUSH_DELAY      5

namespace LP {
    class Serial {
        private:
            int serial_port_fd;
            
            static uint16_t parity;
            static uint16_t stop_bits;
            static uint16_t data_bits;
            static uint16_t flow_ctrl;

            static std::string last_open_port;
            static std::vector<std::string> serial_ports;

            /**
             * @brief Reads the system's available serial ports
             * 
             * @return vector containing the name of the found ports
             */
            static std::vector<std::string> read_system_ports();
        public:
            /**
             * @brief Construct a new Serial object
             * 
             * @param port the name of the serial port to open
             * @param baud the baud rate used for reading the port
             */
            Serial(const char* port, size_t baud);
            ~Serial();

            /**
             * @brief Read from the serial port
             * 
             * @param  buf vector buffer where the read chars will be stored 
             * @return true  if the port's buffer was successfully read
             * @return false if it could not be possible to read the port's buffer or the device was disconnected
             */
            bool read(std::vector<char>& buf) const;

            /**
             * @brief Checks if the port is connected
             * 
             */
            bool is_port_connected() const;

            /**
             * @brief Get the file descriptor object
             * 
             * @return the file descriptor
             */
            int get_file_descriptor() const;

            /**
             * @brief Close the serial port and unlock it
             * 
             */
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
