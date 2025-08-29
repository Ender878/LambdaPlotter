#include <LP/controller.h>
#include <LP/shared.h>
#include <LP/serial.h>
#include <LP/telemetry.h>
#include <LP/toolbar.h>
#include <LP/window.h>
#include <algorithm>
#include <chrono>

#include <cstddef>
#include <cstdio>
#include <exception>
#include <iostream>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

LP::ToolBar     LP::Controller::toolbar;
LP::app_state_t LP::Controller::curr_app_state(IDLE);
LP::app_state_t LP::Controller::prev_app_state(IDLE);
LP::Telemetry   LP::Controller::tel;
LP::PlotView    LP::Controller::plot_view;
std::mutex      LP::Controller::thread_mtx;

void LP::Controller::update() {
    // get available serial ports
    std::vector<std::string> &serial_ports = Serial::get_serial_ports(toolbar.getRefreshButton());

    // update toolbar data
    toolbar.update_serial_ports(serial_ports);
    
    LP::Window::render_toolbar([serial_ports]() {
        toolbar.render(curr_app_state, tel.is_empty(), serial_ports);
        plot_view.render_telemetry(tel);
        plot_view.render_data_format(tel, curr_app_state);
        plot_view.render_plot_options();
    });

    ImVec2 window_size = Window::getWindowSize();
    plot_view.render_plot(tel, curr_app_state, window_size.x * 0.25, 0, window_size.x * 0.75, window_size.y);

    // get updated app state (if we should read or close)
    curr_app_state = toolbar.get_new_app_state(curr_app_state);

    // check if the state is changed (open/close button pressed or device disconnection while reading)
    if (curr_app_state != prev_app_state) {
        if (curr_app_state == READING) {
            start_serial_reading(toolbar.getCurrentPort(), LP::baud_rates[toolbar.getComboboxBaudIndex()].value);
        } else {
            toolbar.setRefreshButton(true);
        }
    }

    if (toolbar.getSaveButton()) {
        save_file();
    }

    if (toolbar.getClearButton()) {
        std::lock_guard<std::mutex> lock(tel.get_data_mtx());
        tel.clear();
        tel.set_start_time();
    }

    prev_app_state = curr_app_state;
}

void LP::Controller::save_file() {
    std::string device_name = Serial::get_last_open_port();
    device_name.erase(0, device_name.find_last_of("/") + 1);
    std::string default_file_name = "bsp_" + device_name + "_" + Telemetry::format_datetime(Telemetry::get_unix_time()) + ".csv";        

    // sanitize default file name (remove ':' from unix timestamp)
    std::replace(default_file_name.begin(), default_file_name.end(), ':', '-');

    std::string path = LP::Window::render_save_fd(default_file_name.c_str());

    if (!path.empty()) {
        tel.dump_data(path, plot_view.get_plot_style().limits, plot_view.get_channels_style(), plot_view.get_plot_style().time_style);
    }
}

void LP::Controller::shutdown() {
    // set app state to IDLE to make sure to close possible device connections
    curr_app_state = IDLE;
}

void LP::Controller::start_serial_reading(std::string port, size_t baud) {
    std::thread([port, baud]() {
        // lock the entire thread to ensure that there won't be any other overlapping serial reading threads
        std::lock_guard<std::mutex> lock(thread_mtx);
        try {

            std::string last_open_port = Serial::get_last_open_port();

            {
                std::lock_guard<std::mutex> lock(tel.get_data_mtx());

                if (port != last_open_port) {
                    tel.clear();
                    tel.set_start_time();
                } else {
                    tel.clear_fragments();
                }
            }

            Serial device(port.c_str(), baud);

            while (curr_app_state == READING) {
                std::vector<char> buffer;

                // if we can't read from the device, there has been a fatal error or it has been disconnected.
                if (device.read(buffer)) {
                    std::string frame_stream = tel.parse_serial(buffer);

                    if (!frame_stream.empty()) {
                        std::lock_guard<std::mutex> lock(tel.get_data_mtx());
                        tel.parse_frame(frame_stream);
                    }
                    
                } else {
                    Serial::set_last_open_port("");
                    curr_app_state = IDLE;
                }

                std::this_thread::sleep_for(std::chrono::milliseconds(THREAD_READ_DELAY));
            }
        } catch (const std::exception& e) {
            std::cerr << "Error while opening port:" << e.what() << std::endl;
            // std::println(stderr, "Error while opening port: {}", e.what());
            curr_app_state = IDLE;
        }
    }).detach();
}
