#include "../common/shared.h"
#include "telemetry.h"
#include <chrono>
#include <cmath>
#include <cstddef>
#include <cstdio>
#include <ctime>
#include <format>
#include <fstream>
#include <iomanip>
#include <mutex>
#include <print>
#include <sstream>
#include <string>
#include <vector>

BSP::Telemetry::Telemetry(size_t t_max_size) 
    : start_time(std::chrono::system_clock::now()), max_size(t_max_size), incomplete_data_chunk("") {}

void BSP::Telemetry::processData(std::vector<char>& buffer) {
    // filtered data chunks to be processed and plotted
    std::string data_frame = "";

    // add '\0` to the end of the buffer to make it a valid string
    buffer.push_back('\0');
    std::string buf_str = buffer.data();

    // iter every character received from the buffer and for every '\n'
    // encountered, add all the characters before it to the `plot_data` string,
    // which will be processed and casted to be ready to be plotted.
    for (const auto &c : buf_str) {
        incomplete_data_chunk += c;

        if (c == '\n') {
            data_frame += incomplete_data_chunk;
            incomplete_data_chunk = "";
        }
    }

    if (!data_frame.empty()) {
        std::stringstream data_frame_stream(data_frame);
        std::string data_chunk;

        // std::println("{}", data_frame);

        // lock the thread while the plot data is updated
        std::lock_guard<std::mutex> lock(BSP::plot_mtx);
        while (std::getline(data_frame_stream, data_chunk, '\n')) {
            dataFormatSpaces(data_chunk);
            
            if (times.size() > max_size) {

                for (auto& stream : values) {
                    stream.second.erase(stream.second.begin());
                }

                times.erase(times.begin());
            }
        }

    }
}

void BSP::Telemetry::dataFormatSpaces(const std::string& data_chunk) {
    std::stringstream data_chunk_stream(data_chunk);
    std::string value;
    size_t i = 1;

    while (std::getline(data_chunk_stream, value, ' ')) {
        try {
            values[i].push_back(std::stod(value));
        } catch (const std::exception &e) {
            values[i].push_back(NAN);
        }

        i++;
    }

    // set the time point only if something has been read
    if (i > 1) {
        times.push_back(getUnixTimestamp());
    }
}

double BSP::Telemetry::getStartTime() const {
    return std::chrono::duration<double>(start_time.time_since_epoch()).count();
}

void BSP::Telemetry::clear() {
    incomplete_data_chunk = "";
    values.clear();
    times.clear();
}

double BSP::Telemetry::getUnixTimestamp() {
    auto now = std::chrono::system_clock::now();

    return std::chrono::duration<double>(now.time_since_epoch()).count();
}

// TODO: save only time window.
void BSP::Telemetry::dump_data(std::string path) const {
    size_t i = 0;
    std::ofstream dump(path);

    if (!dump.is_open()) {
        std::println(stderr, "Error while opening dump file.");
        return;
    }

    std::print(dump, "times");

    std::lock_guard<std::mutex> lock(BSP::plot_mtx);
    for (const auto& el : values) {
        std::print(dump, ";{}", el.first);
    }
    std::println(dump);

    for (const auto& t : times) {
        std::print(dump, "{}", formatUnixTimestamp(t));

        for (const auto& el : values) {
            std::string el_str = "";

            if (i <= times.size() - 1 && i <= el.second.size() - 1) {
                el_str = std::format("{}", el.second.at(i));
            }

            std::print(dump, ";{}", el_str);
        }
        std::println(dump);
        i++;
    }

    std::println("Dump done!!");

    dump.close();
}

std::string BSP::Telemetry::formatUnixTimestamp(double unix_timestamp) {
    // get time from system clock
    time_t time = static_cast<time_t>(unix_timestamp);

    // convert time to `tm` struct
    std::tm tm = *std::localtime(&time);

    // format the string
    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%d_%H:%M:%S");

    return oss.str();
}

bool BSP::Telemetry::is_empty() const {
    std::lock_guard<std::mutex> lock(plot_mtx);

    return values.empty();
}
