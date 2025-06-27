#include "serial.h"
#include <cerrno>
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <fcntl.h>
#include <stdexcept>
#include <termios.h>
#include <unistd.h>

std::string BSP::Serial::last_open_port = "";
std::vector<std::string> BSP::Serial::serial_ports = readSystemPorts();

BSP::Serial::Serial(const char* port, size_t baud) {
    termios tty;

    // get port file descriptor
    serial_port_fd = open(port, O_RDONLY | O_NOCTTY | O_SYNC);

    // check for errors
    if (serial_port_fd < 0) {
        // std::print(stderr, "Error while opening {}: {}", port, strerror(serial_port_fd));
        throw std::runtime_error(strerror(serial_port_fd));
    }

    // read existing settings
    if (tcgetattr(serial_port_fd, &tty) != 0) {
        // std::print(stderr, "Error while reading {} settings: {}", port, strerror(errno));
        throw std::runtime_error(strerror(serial_port_fd));
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
    tty.c_cc[VMIN]  = 1; // wait for at least one character

    // set baud rate
    cfsetspeed(&tty, baud);

    // save tty settings
    if (tcsetattr(serial_port_fd, TCSANOW, &tty) != 0) {
        // std::print(stderr, "Error while saving {} settings: {}", port, strerror(errno));
        throw std::runtime_error(strerror(errno));
    }

    // clear buffer
    tcflush(serial_port_fd, TCIOFLUSH);

    last_open_port = port;
}

BSP::Serial::~Serial() {
    ::close(serial_port_fd);
}

std::vector<std::string>& BSP::Serial::getSerialPorts(bool refresh) {
    if (refresh) {
        serial_ports = readSystemPorts();
    }

    return serial_ports;
}

std::vector<std::string> BSP::Serial::readSystemPorts() {
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

std::string BSP::Serial::getLastOpenPort() {
    return last_open_port;
}

void BSP::Serial::setLastOpenPort(const char* port) {
    last_open_port = port;
}

bool BSP::Serial::read(std::vector<char>& buf) const {
    buf.resize(DEFAULT_BUF_SIZE);

    if (!isPortConnected()) {
        return false;
    }

    int n = ::read(serial_port_fd, buf.data(), buf.size());

    if (n < 0) {
        return false;
    }

    buf.resize(n);

    return true;
}

bool BSP::Serial::isPortConnected() const {
    termios tty;

    // read config to check if the port is still there
    return tcgetattr(serial_port_fd, &tty) == 0;
}

int BSP::Serial::getFileDescriptor() const {
    return serial_port_fd;
}

void BSP::Serial::close() {
    ::close(serial_port_fd);
}
