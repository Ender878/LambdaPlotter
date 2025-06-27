#include "../common/shared.h"
#include "telemetry.h"
#include <chrono>
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

        // lock the thread while the plot data is updated
        std::lock_guard<std::mutex> lock(BSP::plot_mtx);
        while (std::getline(data_frame_stream, data_chunk, '\n')) {
            dataFormatSpaces(data_chunk);

            times.push_back(getUnixTimestamp());
            
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
    size_t i = 0;

    while (std::getline(data_chunk_stream, value, ' ')) {
        try {
            values[i + 1].push_back(std::stod(value));
        } catch (const std::exception &e) {
            std::println(stderr, "Error while processing plot data: {}", e.what());
            clearIncompleteChunk();
        }

        i++;
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

double BSP::Telemetry::getUnixTimestamp() const {
    auto now = std::chrono::system_clock::now();

    return std::chrono::duration<double>(now.time_since_epoch()).count();
}

// TODO: improve and implement Save button
void BSP::Telemetry::dump_data(std::string device_path) const {
    size_t i = 0;

    std::string home = std::getenv("HOME");
    std::string datetime = getCurrDateTime();
    std::string device = device_path.erase(0, device_path.find_last_of("/"));

    std::ofstream dump(home + LOG_PATH + "/" + device + "_" + datetime + ".csv");

    if (!dump.is_open()) {
        std::println(stderr, "Error while opening dump file.");
        return;
    }

    std::print(dump, "times");

    for (const auto& el : values) {
        std::print(dump, ";{}", el.first);
    }
    std::println(dump);

    for (const auto& t : times) {
        std::print(dump, "{}", t);

        for (const auto& el : values) {
            std::string el_str = "";

            if (i < times.size() - 1 && i < el.second.size() - 1) {
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

std::string BSP::Telemetry::getCurrDateTime() const {
    // get time from system clock
    time_t now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

    // convert time to `tm` struct
    std::tm tm = *std::localtime(&now);

    // format the string
    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%d_%H:%M:%S");

    return oss.str();
}
