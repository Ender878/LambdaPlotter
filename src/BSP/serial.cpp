#include "serial.h"
#include "../common/defines.h"
#include <cerrno>
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <print>

#include <fcntl.h> // Contains file controls like O_RDWR
#include <termios.h>
#include <unistd.h>

BSP::Serial::Serial() {
    m_serial_ports = getAvailablePorts();
    serial_port = -1;
}

BSP::Serial::~Serial() {
    ::close(serial_port);
}

std::vector<std::string>& BSP::Serial::getSerialPorts(bool refresh) {
    if (refresh) {
        m_serial_ports = getAvailablePorts();
    }

    return m_serial_ports;
}

std::vector<std::string> BSP::Serial::getAvailablePorts() {
    std::vector<std::string> ports;

    for (const auto& entry : std::filesystem::directory_iterator("/dev/")) {
        auto entry_str = entry.path().string();

        if (entry_str.find("ttyACM") != std::string::npos ||
            entry_str.find("ttyUSB") != std::string::npos) 
        {
            ports.push_back(entry_str);
        } 
    }

    return ports;
}

bool BSP::Serial::configurePort(size_t t_port_index, size_t t_baud_index) {
    const char* port_str = m_serial_ports[t_port_index].c_str();
    termios tty;

    // if a serial port is already open, close it 
    if (serial_port >= 0) {
        ::close(serial_port);
    }

    // get port file descriptor
    serial_port = open(port_str, O_RDWR | O_NOCTTY | O_SYNC);

    // check for errors
    if (serial_port < 0) {
        std::print(stderr, "Error while opening {}: {}", port_str, strerror(serial_port));
        return false;
    }

    // read existing settings
    if (tcgetattr(serial_port, &tty) != 0) {
        std::print(stderr, "Error while reading {} settings: {}", port_str, strerror(errno));
        return false;
    }

    // parity bit
    tty.c_cflag &= ~PARENB; // Clear parity bit, disabling parity (most common)
    // tty.c_cflag |= PARENB;  // Set parity bit, enabling parity

    // stop bits
    tty.c_cflag &= ~CSTOPB; // Clear stop field, only one stop bit used in communication (most common)
    // tty.c_cflag |= CSTOPB;  // Set stop field, two stop bits used in communication

    // data bits
    tty.c_cflag &= ~CSIZE; // Clear all the size bits, then use one of the statements below
    // tty.c_cflag |= CS5; // 5 bits per byte
    // tty.c_cflag |= CS6; // 6 bits per byte
    // tty.c_cflag |= CS7; // 7 bits per byte
    tty.c_cflag |= CS8; // 8 bits per byte (most common)

    // hardware flow control
    tty.c_cflag &= ~CRTSCTS; // Disable RTS/CTS hardware flow control (most common)
    // tty.c_cflag |= CRTSCTS;  // Enable RTS/CTS hardware flow control

    // CREAD and CLOCAL
    tty.c_cflag |= CREAD | CLOCAL; // Turn on READ & ignore ctrl lines (CLOCAL)

    // disable canonical mode (read only when new line, not ideal for serial communications)
    tty.c_lflag &= ~ICANON;

    tty.c_lflag &= ~ECHO; // Disable echo
    tty.c_lflag &= ~ECHOE; // Disable erasure
    tty.c_lflag &= ~ECHONL; // Disable new-line echo

    // disable signal chars 
    tty.c_lflag &= ~ISIG; // Disable interpretation of INTR, QUIT and SUSP

    // disable software flow control
    tty.c_iflag &= ~(IXON | IXOFF | IXANY); // Turn off s/w flow ctrl
    tty.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL); // Disable any special handling of received bytes

    tty.c_cc[VTIME] = 1; // timeout of 1 decisecond
    tty.c_cc[VMIN]  = 0; // return immediately if no data is available

    // set baud rate
    cfsetspeed(&tty, baud_rates[t_baud_index].value);

    // save tty settings
    if (tcsetattr(serial_port, TCSANOW, &tty) != 0) {
        std::print(stderr, "Error while saving {} settings: {}", port_str, strerror(errno));
        return false;
    }

    return true;
}

bool BSP::Serial::read(std::vector<char>& buf) const {
    buf.resize(DEFAULT_BUF_SIZE);

    if (!isPortConnected()) {
        return false;
    }

    int n = ::read(serial_port, buf.data(), buf.size());

    if (n < 0) {
        return false;
    }

    buf.resize(n);

    return true;
}

bool BSP::Serial::isPortConnected() const {
    termios tty;

    // read config to see if the port is still there
    return tcgetattr(serial_port, &tty) == 0;
}

void BSP::Serial::close() {
    ::close(serial_port);
}

namespace BSP {
    const baud_rate_t baud_rates[] = {
        {"300",     B300},
        {"600",     B600},
        {"1200",    B1200},
        {"1800",    B1800},
        {"2400",    B2400},
        {"4800",    B4800},
        {"9600",    B9600},
        {"19200",   B19200},
        {"38400",   B38400},
        {"57600",   B57600},
        {"115200", B115200}
    };

    constexpr size_t baud_rates_size = sizeof(baud_rates) / sizeof(*baud_rates);

    Serial serial;
}
