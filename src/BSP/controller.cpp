#include "controller.h"
#include "../common/defines.h"
#include "app.h"
#include "serial.h"
#include "telemetry.h"
#include <algorithm>
#include <chrono>

#include <cstdio>
#include <deque>
#include <exception>
#include <iterator>
#include <mutex>
#include <optional>
#include <print>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

std::optional<size_t> BSP::Controller::combobox_port_index(std::nullopt);
size_t BSP::Controller::combobox_baud_index = 0;
bool BSP::Controller::button_status = false;
std::atomic<bool> BSP::Controller::should_read(false);
std::atomic<bool> BSP::Controller::refresh_ports(false);
std::string BSP::Controller::current_port("");
std::deque<double> BSP::Controller::plot_data_list;
std::deque<long> BSP::Controller::plot_timestamps;
long BSP::Controller::plot_begin_time = 0;

std::string BSP::Controller::last_open_port = "";
std::string BSP::Controller::prev_open_port = "";
std::string BSP::Controller::incomplete_data_chunk = "";      // static string storing incomplete buffer data chunks (chunks missing the final \n)

void BSP::Controller::update(Serial& serial) {
    std::vector<std::string>& serial_ports = serial.getSerialPorts(refresh_ports.load());

    if (refresh_ports.load()) {
        
        // if the port list is empty, but the list index has a value, set the index to null
        if (serial_ports.empty() && combobox_port_index.has_value()) {
            combobox_port_index = std::nullopt;
        } else if (!serial_ports.empty() && combobox_port_index.has_value()) {
            // if the port list is not empty and the list index is set, check if the pointed value exists in the list and reassing the correct index
            // if the pointed element no longer exist, set the index to null
            if (std::any_of(serial_ports.begin(), serial_ports.end(), [](std::string v) { return v == current_port; })) {
                combobox_port_index = std::distance(serial_ports.begin(), std::find(serial_ports.begin(), serial_ports.end(), current_port)); 
            } else {
                combobox_port_index = std::nullopt;
            }
        }

    }

    // get current port
    current_port = combobox_port_index.has_value() ? serial_ports[combobox_port_index.value()] : "";

    BSP::Window::renderMenuBar(serial_ports, combobox_port_index, BSP::baud_rates, combobox_baud_index, button_status, should_read, refresh_ports);

    // check if the open/close port button has been pressed
    if (button_status) {
        // if we are not already reading and there is a port selected, start reading.
        // If we are reading and the button has been pressed, stop reading.
        if (combobox_port_index.has_value() && !should_read.load()) {                        
            std::println("Opening port {} with baud rate {}", current_port, BSP::baud_rates[combobox_baud_index].str);
            last_open_port = current_port;
            incomplete_data_chunk = "";

            serial.configurePort(combobox_port_index.value(), combobox_baud_index);

            startSerialReading(serial);
        } else if (should_read.load()) {
            serial.close();
            should_read.store(false);

            {
                std::lock_guard<std::mutex> lock(mtx);

                prev_open_port = last_open_port;
                incomplete_data_chunk = "";
            }
        }
    }

    BSP::Window::renderPlot(plot_data_list, plot_timestamps);
}

void BSP::Controller::startSerialReading(Serial& s) {
    should_read = true;

    if (last_open_port != prev_open_port) {
        clearPlotData();
        plot_begin_time = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    }

    std::thread([&s]() {

        while (should_read) {
            std::vector<char> buf;

            if (s.read(buf)) {
                //std::print("{}", buf.data());
                processData(buf);
            } else {
                // if the device get disconnected or we can't read, stop reading and refresh the port list
                s.close();
                should_read.store(false);
                refresh_ports.store(true);
                
                {
                    std::lock_guard<std::mutex> lock(mtx);

                    prev_open_port = "";
                    incomplete_data_chunk = "";
                }
            }
 
            std::this_thread::sleep_for(std::chrono::milliseconds(THREAD_READ_DELAY));
        }

    }).detach();
}

// TODO: implement multiple data plotting & implement plot clearing
void BSP::Controller::processData(std::vector<char> buffer) {
    std::vector<double> buf_data_stream;                // vector storing plot data read from the buffer
    std::string plot_data = "";                         // filtered data chunks to be processed and plotted

    buffer.push_back('\0');

    std::string buf_str = buffer.data();

    // iter every character received from the buffer and for every '\n' encountered,
    // add all the characters before it to the `plot_data` string, which will be processed
    // and casted to be ready to be plotted.
    for (const auto& c : buf_str) {
        incomplete_data_chunk += c;

        if (c == '\n') {
            plot_data += incomplete_data_chunk;
            incomplete_data_chunk = "";
        }
    }

    if (!plot_data.empty()) {
        std::stringstream str_stream(plot_data);
        std::string data_fragment;
    
        while (std::getline(str_stream, data_fragment, '\n')) {
            try {
                //std::println("{}", data_fragment);
                buf_data_stream.push_back(std::stod(data_fragment));
            } catch (const std::exception& e) {
                std::println(stderr, "Error while processing plot data: {}", e.what());
            }
        }

        {
            std::lock_guard<std::mutex> lock(mtx);
            // append new data to the plot buffer
            for (const auto& val : buf_data_stream) {

                plot_data_list.push_back(val);
                plot_timestamps.push_back(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count() - plot_begin_time);
        
                if (plot_timestamps.back() > PLOT_TIME_WINDOW) {
                    plot_data_list.pop_front();
                    plot_timestamps.pop_front();
                }
            }
        }
        
    }
}

void BSP::Controller::clearPlotData() {
    std::lock_guard<std::mutex> lock(mtx);

    plot_data_list.clear();
    plot_timestamps.clear();
}
