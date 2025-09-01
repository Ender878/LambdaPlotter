#include <LP/serial.h>
#include <cerrno>
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <format>
#include <stdexcept>
#include <thread>

#ifndef _WIN32
#include <fcntl.h>
#include <filesystem>
#include <sys/file.h>
#include <unistd.h>
#endif

std::string              LP::Serial::last_open_port = "";
std::vector<std::string> LP::Serial::serial_ports   = read_system_ports();

uint16_t LP::Serial::parity    = 0;      // parity bit disabled
uint16_t LP::Serial::stop_bits = 1;      // 1 stop bit
uint16_t LP::Serial::data_bits = LP_CS8; // 8 data bits
uint16_t LP::Serial::flow_ctrl = 0;      // hardware flow control disabled

#ifndef _WIN32
LP::Serial::Serial(const char *port, size_t baud) {
    termios tty;

    // get port file descriptor
    serial_port_fd = open(port, O_RDWR | O_NOCTTY);

    // check for errors
    if (serial_port_fd < 0) {
        // std::print(stderr, "Error while opening {}: {}", port, strerror(serial_port_fd));
        throw std::runtime_error(
            std::format("Error while opening {}: {}", port, strerror(serial_port_fd)));
    }

    // lock the port to prevent concurrent reading/writing
    if (flock(serial_port_fd, LOCK_EX | LOCK_NB) == -1) {
        ::close(serial_port_fd);
        throw std::runtime_error(std::format("{} is already in use!", port));
    }

    // read existing settings
    if (tcgetattr(serial_port_fd, &tty) != 0) {
        // std::print(stderr, "Error while reading {} settings: {}", port, strerror(errno));
        throw std::runtime_error(
            std::format("Error while reading {} settings: {}", port, strerror(errno)));
    }

    // parity
    // - disabled
    // - even
    // - odd
    if (parity == 1) {
        tty.c_cflag |= PARENB; // Set parity bit, enabling parity (EVEN)
        tty.c_iflag |= INPCK;  // Enable parity checking
    } else if (parity == 2) {
        tty.c_cflag |= PARENB | PARODD; // Set parity bit, enabling parity (ODD)
        tty.c_iflag |= INPCK;           // Enable parity checking
    } else {
        tty.c_cflag &= ~PARENB; // Clear parity bit, disabling parity (most common)
        tty.c_iflag &= ~INPCK;  // Disable parity checking
    }

    // stop bits
    if (stop_bits == 1) {
        tty.c_cflag &=
            ~CSTOPB; // Clear stop field, only one stop bit used in communication (most common)
    } else {
        tty.c_cflag |= CSTOPB; // Set stop field, two stop bits used in communication
    }

    // data bits (n << 4)
    tty.c_cflag &= ~CSIZE; // Clear all the size bits, then use one of the statements below
    // tty.c_cflag |= CS5; // 5 bits per byte
    // tty.c_cflag |= CS6; // 6 bits per byte
    // tty.c_cflag |= CS7; // 7 bits per byte
    // tty.c_cflag |= CS8; // 8 bits per byte (most common)
    tty.c_cflag |= (data_bits << 4);

    // flow control
    if (flow_ctrl == 1) {
        tty.c_cflag |= CRTSCTS; // Enable RTS/CTS hardware flow control
        tty.c_iflag &= ~(IXON | IXOFF | IXANY);
    } else if (flow_ctrl == 2) {
        tty.c_iflag |= (IXON | IXOFF | IXANY); // enable software flow control
        tty.c_cflag &= ~CRTSCTS;
    } else {
        tty.c_cflag &= ~CRTSCTS; // Disable RTS/CTS hardware flow control (most common)
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
    tty.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR |
                     ICRNL); // Disable any special handling of received bytes

    tty.c_cc[VTIME] = 0;
    tty.c_cc[VMIN]  = 1; // wait for at least one byte

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

std::vector<std::string> LP::Serial::read_system_ports() {
    std::vector<std::string> ports;

    for (const auto &entry : std::filesystem::directory_iterator("/dev/")) {
        auto entry_str = entry.path().string();

        if (entry_str.find("ttyACM") != std::string::npos ||
            entry_str.find("ttyUSB") != std::string::npos) {
            ports.push_back(entry_str);
        }
    }

    return ports;
}

bool LP::Serial::read(std::vector<char> &buf) const {
    buf.resize(DEFAULT_BUF_SIZE);

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

int LP::Serial::get_file_descriptor() const { return serial_port_fd; }

void LP::Serial::close() { ::close(serial_port_fd); }

#else

LP::Serial::Serial(const char *port, size_t baud) {
    // === Open the serial port ===
    serial_port_handle = CreateFileA(port,                         // port name
                                     GENERIC_READ | GENERIC_WRITE, // enable read/write
                                     0,                            // exclusive access
                                     NULL,
                                     OPEN_EXISTING, // open existing port
                                     0,             // non overlapped IO
                                     NULL);

    if (serial_port_handle == INVALID_HANDLE_VALUE) {
        throw std::runtime_error(std::format("Error while opening {}: {}", port, GetLastError()));
    }

    // === Configure serial port parameters ===
    DCB serial_params       = {0}; // zero-out the DCB (device control block) structure
    serial_params.DCBlength = sizeof(serial_params); // set the struct's size

    // Get current serial port's settings
    if (!GetCommState(serial_port_handle, &serial_params)) {
        CloseHandle(serial_port_handle);
        throw std::runtime_error("Error getting serial port state");
    }

    // Configure the port with user's settings
    serial_params.BaudRate = baud;
    serial_params.ByteSize = 5 + data_bits;
    serial_params.StopBits = (stop_bits == 2) ? TWOSTOPBITS : ONESTOPBIT;
    serial_params.Parity   = (parity) ? (parity == 1 ? EVENPARITY : ODDPARITY) : NOPARITY;

    // Configure flow control
    if (flow_ctrl == 1) { // Hardware flow control
        serial_params.fOutxCtsFlow = TRUE;
        serial_params.fRtsControl  = RTS_CONTROL_HANDSHAKE;
        serial_params.fOutX        = FALSE;
        serial_params.fInX         = FALSE;
    } else if (flow_ctrl == 2) { // Software flow control
        serial_params.fOutxCtsFlow = FALSE;
        serial_params.fRtsControl  = RTS_CONTROL_ENABLE;
        serial_params.fOutX        = TRUE;
        serial_params.fInX         = TRUE;
    } else { // No flow control
        serial_params.fOutxCtsFlow = FALSE;
        serial_params.fRtsControl  = RTS_CONTROL_ENABLE;
        serial_params.fOutX        = FALSE;
        serial_params.fInX         = FALSE;
    }

    // Save the new settings
    if (!SetCommState(serial_port_handle, &serial_params)) {
        CloseHandle(serial_port_handle);
        throw std::runtime_error("Error setting serial port state");
    }

    // Set timeouts
    COMMTIMEOUTS timeouts               = {0};
    timeouts.ReadIntervalTimeout        = MAXDWORD; // Max time between bytes (ms)
    timeouts.ReadTotalTimeoutMultiplier = 0;        // Per-byte timeout
    timeouts.ReadTotalTimeoutConstant   = 0;        // Constant timeout (ms)

    // set timeouts settings
    if (!SetCommTimeouts(serial_port_handle, &timeouts)) {
        CloseHandle(serial_port_handle);
        throw std::runtime_error("Error setting serial port timeouts");
    }

    // clear serial buffer
    std::this_thread::sleep_for(std::chrono::milliseconds(FLUSH_DELAY));
    PurgeComm(serial_port_handle, PURGE_RXCLEAR | PURGE_TXCLEAR);
    last_open_port = port;
}

LP::Serial::~Serial() { CloseHandle(serial_port_handle); }

std::vector<std::string> LP::Serial::read_system_ports() {
    std::vector<std::string> ports;
    char                     target_path[255];

    for (int i = 1; i < 256; i++) {
        std::string com_name = "COM" + std::to_string(i);
        if (QueryDosDeviceA(com_name.c_str(), target_path, 255) != 0) {
            ports.push_back(com_name);
        }
    }
    return ports;
}

bool LP::Serial::read(std::vector<char> &buf) const {
    buf.resize(DEFAULT_BUF_SIZE);
    DWORD bytesRead;

    if (!ReadFile(serial_port_handle, buf.data(), buf.size(), &bytesRead, NULL)) {
        return false;
    }

    buf.resize(bytesRead);
    return true;
}

bool LP::Serial::is_port_connected() const {
    // read port settings to check if it is still there
    DCB serial_params       = {0};
    serial_params.DCBlength = sizeof(serial_params);

    return GetCommState(serial_port_handle, &serial_params);
}

void LP::Serial::close() { CloseHandle(serial_port_handle); }
#endif

std::vector<std::string> &LP::Serial::get_serial_ports(bool refresh) {
    if (refresh) {
        serial_ports = read_system_ports();
    }

    return serial_ports;
}

std::string LP::Serial::get_last_open_port() { return last_open_port; }

void LP::Serial::set_last_open_port(const char *port) { last_open_port = port; }
