#include <LP/serial.h>
#include <cerrno>
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <fcntl.h>
#include <format>
#include <thread>
#include <sys/file.h>
#include <stdexcept>
#include <termios.h>
#include <unistd.h>

std::string LP::Serial::last_open_port = "";
std::vector<std::string> LP::Serial::serial_ports = read_system_ports();

uint16_t LP::Serial::parity    = 0;         // parity bit disabled
uint16_t LP::Serial::stop_bits = 1;         // 1 stop bit
uint16_t LP::Serial::data_bits = CS8;       // 8 data bits
uint16_t LP::Serial::flow_ctrl = 0;         // hardware flow control disabled

LP::Serial::Serial(const char* port, size_t baud) {
    termios tty;

    // get port file descriptor
    serial_port_fd = open(port, O_RDWR | O_NOCTTY);

    // check for errors
    if (serial_port_fd < 0) {
        // std::print(stderr, "Error while opening {}: {}", port, strerror(serial_port_fd));
        throw std::runtime_error(std::format("Error while opening {}: {}", port, strerror(serial_port_fd)));
    }

    // lock the port to prevent concurrent reading/writing
    if (flock(serial_port_fd, LOCK_EX | LOCK_NB) == -1) {
        ::close(serial_port_fd);
        throw std::runtime_error(std::format("{} is already in use!", port));
    }

    // read existing settings
    if (tcgetattr(serial_port_fd, &tty) != 0) {
        // std::print(stderr, "Error while reading {} settings: {}", port, strerror(errno));
        throw std::runtime_error(std::format("Error while reading {} settings: {}", port, strerror(errno)));
    }

    // parity
    // - disabled
    // - even
    // - odd
    if (parity)  {
        tty.c_cflag |= parity;      // Set parity bit, enabling parity
        tty.c_iflag |= INPCK;       // Enable parity checking
    } else {
        tty.c_cflag &= ~PARENB;     // Clear parity bit, disabling parity (most common)
        tty.c_iflag &= ~INPCK;      // Disable parity checking
    }

    // stop bits
    if (stop_bits == 1) {
        tty.c_cflag &= ~CSTOPB;    // Clear stop field, only one stop bit used in communication (most common)
    } else {
        tty.c_cflag |= CSTOPB;     // Set stop field, two stop bits used in communication
    }
    
    // data bits (n << 4)
    tty.c_cflag &= ~CSIZE; // Clear all the size bits, then use one of the statements below
    // tty.c_cflag |= CS5; // 5 bits per byte
    // tty.c_cflag |= CS6; // 6 bits per byte
    // tty.c_cflag |= CS7; // 7 bits per byte
    // tty.c_cflag |= CS8; // 8 bits per byte (most common)
    tty.c_cflag |= data_bits;

    // flow control
    if (flow_ctrl == 1) {
        tty.c_cflag |= CRTSCTS;                 // Enable RTS/CTS hardware flow control
        tty.c_iflag &= ~(IXON | IXOFF | IXANY);
    } else if (flow_ctrl == 2) {
        tty.c_iflag |= (IXON | IXOFF | IXANY);  // enable software flow control
        tty.c_cflag &= ~CRTSCTS;
    } else {
        tty.c_cflag &= ~CRTSCTS;                // Disable RTS/CTS hardware flow control (most common)
        tty.c_iflag &= ~(IXON | IXOFF | IXANY); // disable software flow control
    }

    // CREAD and CLOCAL
    tty.c_cflag |= CREAD | CLOCAL; // Turn on READ & ignore ctrl lines (CLOCAL)

    // == LOCAL FLAGS ==
    // Local flags are used to control how the terminal processes characters.
    // disable canonical mode (read only when new line, not ideal for serial communications)
    tty.c_lflag &= ~ICANON;

    tty.c_lflag &= ~ECHO;   // Disable echo
    tty.c_lflag &= ~ECHOE;  // Disable erasure
    tty.c_lflag &= ~ECHONL; // Disable new-line echo

    // disable signal chars 
    tty.c_lflag &= ~ISIG; // Disable interpretation of INTR, QUIT and SUSP

    // == INPUT FLAGS ==
    tty.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL); // Disable any special handling of received bytes

    tty.c_cc[VTIME] = 1; // timeout of 1 decisecond
    tty.c_cc[VMIN]  = 1; // wait for at least one character

    // set baud rate
    cfsetspeed(&tty, baud);

    // save tty settings
    if (tcsetattr(serial_port_fd, TCSANOW, &tty) != 0) {
        throw std::runtime_error(strerror(errno));
    }

    // clear serial buffer
    std::this_thread::sleep_for(std::chrono::milliseconds(FLUSH_DELAY));
    tcflush(serial_port_fd, TCIOFLUSH);

    last_open_port = port;
}

LP::Serial::~Serial() {
    // unlock and close the serial port
    flock(serial_port_fd, LOCK_UN);
    ::close(serial_port_fd);
}

std::vector<std::string>& LP::Serial::get_serial_ports(bool refresh) {
    if (refresh) {
        serial_ports = read_system_ports();
    }

    return serial_ports;
}

std::vector<std::string> LP::Serial::read_system_ports() {
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

std::string LP::Serial::get_last_open_port() {
    return last_open_port;
}

void LP::Serial::set_last_open_port(const char* port) {
    last_open_port = port;
}

bool LP::Serial::read(std::vector<char>& buf) const {
    buf.resize(DEFAULT_BUF_SIZE);

    if (!is_port_connected()) {
        return false;
    }

    int n = ::read(serial_port_fd, buf.data(), buf.size());

    if (n < 0) {
        return false;
    }

    buf.resize(n);

    return true;
}

bool LP::Serial::is_port_connected() const {
    termios tty;

    // read config to check if the port is still there
    return tcgetattr(serial_port_fd, &tty) == 0;
}

int LP::Serial::get_file_descriptor() const {
    return serial_port_fd;
}

void LP::Serial::close() {
    ::close(serial_port_fd);
}
