#include <BSP/controller.h>
#include <common/shared.h>
#include <BSP/serial.h>
#include <BSP/telemetry.h>
#include <BSP/toolbar.h>
#include <BSP/window.h>
#include <algorithm>
#include <chrono>

#include <cstddef>
#include <cstdio>
#include <exception>
#include <mutex>
#include <print>
#include <string>
#include <thread>
#include <vector>

BSP::ToolBar BSP::Controller::toolbar;
BSP::app_state_t BSP::Controller::curr_app_state(IDLE);
BSP::app_state_t BSP::Controller::prev_app_state(IDLE);
BSP::Telemetry BSP::Controller::tel;
BSP::PlotView BSP::Controller::plot_view;

void BSP::Controller::update() {
    // get available serial ports
    std::vector<std::string> &serial_ports = Serial::getSerialPorts(toolbar.getRefreshButton());

    // update toolbar data
    toolbar.updateSerialPorts(serial_ports);
    
    BSP::Window::renderMenuBar([serial_ports]() {
        toolbar.render(curr_app_state, tel.is_empty(), serial_ports);
        plot_view.renderTelemetryToolbar(tel);
        plot_view.renderDataFormat(tel, curr_app_state);
        plot_view.renderPlotOptions();
    });

    ImVec2 window_size = Window::getWindowSize();
    plot_view.renderPlot(tel, curr_app_state, window_size.x * 0.25, 0, window_size.x * 0.75, window_size.y);

    // get updated app state (if we should read or close)
    curr_app_state = toolbar.getAppState(curr_app_state);

    // check if the state is changed (open/close button pressed or device disconnection while reading)
    if (curr_app_state != prev_app_state) {
        if (curr_app_state == READING) {
            startSerialReading(toolbar.getCurrentPort(), BSP::baud_rates[toolbar.getComboboxBaudIndex()].value);
        } else {
            toolbar.setRefreshButton(true);
        }
    }

    if (toolbar.getSaveButton()) {
        saveFile();
    }

    if (toolbar.getClearButton()) {
        std::lock_guard<std::mutex> lock(plot_mtx);
        tel.clear();
        tel.set_start_time();
    }

    prev_app_state = curr_app_state;
}

void BSP::Controller::saveFile() {
    std::string device_name = Serial::getLastOpenPort();
    device_name.erase(0, device_name.find_last_of("/") + 1);
    std::string default_file_name = "bsp_" + device_name + "_" + Telemetry::formatUnixTimestamp(Telemetry::get_unix_time()) + ".csv";        

    // sanitize default file name (remove ':' from unix timestamp)
    std::replace(default_file_name.begin(), default_file_name.end(), ':', '-');

    std::string path = BSP::Window::saveFileDialog(default_file_name.c_str());

    if (!path.empty()) {
        tel.dump_data(path, plot_view.getPlotStyle().limits, plot_view.getChannelStyles(), plot_view.getPlotStyle().time_style);
    }
}

void BSP::Controller::shutdown() {
    // set app state to IDLE to make sure to close possible device connections
    curr_app_state = IDLE;
}

void BSP::Controller::startSerialReading(std::string port, size_t baud) {
    std::thread([port, baud]() {
        // lock the entire thread to ensure that there won't be any other overlapping reading threads
        std::lock_guard<std::mutex> lock(BSP::thread_mtx);
        try {
            std::string last_open_port = Serial::getLastOpenPort();

            {
                std::lock_guard<std::mutex> lock(BSP::plot_mtx);

                if (port != last_open_port) {
                    tel.clear();
                    tel.set_start_time();
                } else {
                    tel.clearIncompleteChunk();
                }
            }

            Serial device(port.c_str(), baud);

            while (curr_app_state == READING) {
                std::vector<char> buffer;

                // if we can't read from the device, there has been a fatal error or it has been disconnected.
                if (device.read(buffer)) {
                    std::string frame_stream = tel.parse_serial(buffer);

                    if (!frame_stream.empty()) {
                        std::lock_guard<std::mutex> lock(BSP::plot_mtx);
                        tel.parse_frame(frame_stream);
                    }
                    
                } else {
                    Serial::setLastOpenPort("");
                    curr_app_state = IDLE;
                }

                std::this_thread::sleep_for(std::chrono::milliseconds(THREAD_READ_DELAY));
            }
        } catch (const std::exception& e) {
            std::println(stderr, "Error while opening port: {}", e.what());
            curr_app_state = IDLE;
        }
    }).detach();
}
