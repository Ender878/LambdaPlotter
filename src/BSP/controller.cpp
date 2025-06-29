#include "controller.h"
#include "../common/shared.h"
#include "serial.h"
#include "telemetry.h"
#include "toolbar.h"
#include "window.h"
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

void BSP::Controller::update() {
    // get available serial ports
    std::vector<std::string> &serial_ports = Serial::getSerialPorts(toolbar.getRefreshButton());

    // update toolbar data
    toolbar.updateSerialPorts(serial_ports);

    // render toolbar with new data
    BSP::Window::renderToolBar(toolbar, serial_ports, curr_app_state, tel.is_empty());

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
        tel.setStartTime();
    }

    BSP::Window::renderPlot(tel, toolbar.getComboboxTimeIndex(), curr_app_state);

    prev_app_state = curr_app_state;
}

void BSP::Controller::saveFile() {
    std::string device_name = Serial::getLastOpenPort();
    device_name.erase(0, device_name.find_last_of("/") + 1);
    std::string default_file_name = "bsp_" + device_name + "_" + Telemetry::formatUnixTimestamp(Telemetry::getUnixTimestamp()) + ".csv";        

    // sanitize default file name (remove ':' from unix timestamp)
    std::replace(default_file_name.begin(), default_file_name.end(), ':', '-');

    std::string path = BSP::Window::saveFileDialog(default_file_name.c_str());

    if (!path.empty()) {
        tel.dump_data(path);
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
                    tel.setStartTime();
                } else {
                    tel.clearIncompleteChunk();
                }
            }

            Serial device(port.c_str(), baud);

            while (curr_app_state == READING) {
                std::vector<char> buffer;

                // if we can't read from the device, there has been a fatal error or it has been disconnected.
                if (device.read(buffer)) {
                    tel.processData(buffer);
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
