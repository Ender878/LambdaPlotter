#include <algorithm>
#include <common/shared.h>
#include <BSP/telemetry.h>
#include <chrono>
#include <cmath>
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <format>
#include <fstream>
#include <iomanip>
#include <iterator>
#include <mutex>
#include <print>
#include <regex>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

BSP::Telemetry::Telemetry(size_t t_max_size) 
    : start_time(std::chrono::system_clock::now()), max_size(t_max_size), frame_fragments("") {}

std::string BSP::Telemetry::parse_serial(std::vector<char>& buffer) {
    // char buffer to valid string
    buffer.push_back('\0');
    std::string buffer_str = buffer.data();

    // valid frames found from the read serial buffer
    std::string frame_stream = "";

    // last frame end token index
    size_t last_frame_end_i  = 0;

    // apply special characters inserted by the user 
    std::string frame_end = Telemetry::format_special_chars(frame_format.frame_end);
    size_t frame_end_len  = frame_end.length();

    do {
        // search for the frame end token
        size_t frame_end_i = buffer_str.find(frame_end, last_frame_end_i);

        // if it has been found, there is a valid frame. Update frame_stream.
        // if there is no token, append the characters to the fragment string
        if (frame_end_i != std::string::npos) {
            frame_stream    += frame_fragments + buffer_str.substr(last_frame_end_i, frame_end_i - last_frame_end_i + frame_end_len);
            frame_fragments = "";
        } else {
            frame_fragments += buffer_str.substr(last_frame_end_i, buffer_str.length());
        }

        last_frame_end_i = (frame_end_i == std::string::npos) 
            ? frame_end_i 
            : frame_end_i + frame_end_len;
    } while (last_frame_end_i != std::string::npos);

    return frame_stream;
}

void BSP::Telemetry::parse_frame(const std::string& frame_stream) {
    std::string frame_end   = Telemetry::format_special_chars(frame_format.frame_end);
    std::string channel_sep = Telemetry::format_special_chars(frame_format.channel_sep);
    std::string name_sep    = Telemetry::format_special_chars(frame_format.name_sep);

    // calc token length
    size_t frame_end_len    = frame_end.length();
    size_t channel_end_len  = channel_sep.length();
    size_t name_sep_len     = name_sep.length();

    // last frame end token index
    size_t last_frame_end_i = 0;

    do {
        // find the next frame end token
        size_t frame_end_i = frame_stream.find(frame_end, last_frame_end_i);

        // if found, get the current frame
        if (frame_end_i != std::string::npos) {
            std::string frame = frame_stream.substr(last_frame_end_i, frame_end_i - last_frame_end_i);

            // last channel sep token index
            size_t last_channel_sep_i = 0;
            // channel id
            size_t id = 1;

            do {
                // find the next channel separator
                size_t channel_sep_i = frame.find(channel_sep, last_channel_sep_i);
                
                // get the channel
                std::string channel = frame.substr(last_channel_sep_i, (channel_sep_i != std::string::npos ? channel_sep_i : frame.length()) - last_channel_sep_i);
                if (!channel.empty()) {
                    // for default, the value of the channel is the channel itself,
                    // this will change if the `named` field of the frame format is active.
                    std::string value = channel;
                    std::string name  = std::format("Data {}", id);

                    if (frame_format.named) {
                        // if the frame is named, read and find the name and the actual value
                        size_t name_sep_i = channel.find(name_sep);

                        // if for some reason the name separator is not present,
                        // use the default values for `name` and `value`
                        if (name_sep_i != std::string::npos) {
                            name  = channel.substr(0, name_sep_i);
                            value = channel.substr(name_sep_i + name_sep_len, channel.length());
                        }
                    }

                    // push the default channel attributes
                    // if they are not set
                    if (!data.contains(id)) {
                        data[id].name   = name;
                        data[id].scale  = 1.0f;
                        data[id].offset = 0.0f;
                    }
                    
                    // push the value
                    try {
                        data[id].values.push_back(std::stod(value) * data[id].scale + data[id].offset);
                    } catch (const std::invalid_argument &e) {
                        data[id].values.push_back(NAN);
                    }

                    id++;
                }

                last_channel_sep_i = (channel_sep_i == std::string::npos)
                    ? channel_sep_i
                    : channel_sep_i + channel_end_len;
            } while (last_channel_sep_i != std::string::npos);

            // set the time point only if something has been read
            if (id > 1) {
                times_unix.push_back(get_unix_time());
                times_elapsed.push_back(get_elapsed_time());
            }
        }

        last_frame_end_i = (frame_end_i == std::string::npos)
            ? frame_end_i
            : frame_end_i + frame_end_len;
    } while (last_frame_end_i != std::string::npos);
}

std::string BSP::Telemetry::format_special_chars(const char* s) {
    std::string result = s;

    result = std::regex_replace(result, std::regex(R"(\\n)"), "\n");
    result = std::regex_replace(result, std::regex(R"(\\r)"), "\r");
    result = std::regex_replace(result, std::regex(R"(\\t)"), "\t");
    result = std::regex_replace(result, std::regex(R"(\\f)"), "\f");
    result = std::regex_replace(result, std::regex(R"(\\v)"), "\v");

    return result;
}

int BSP::Telemetry::get_elapsed_time() {
    double now = get_unix_time();
    double start_time = get_start_time();

    return int((now - start_time) * 1000);
}

double BSP::Telemetry::get_start_time() const {
    return std::chrono::duration<double>(start_time.time_since_epoch()).count();
}

void BSP::Telemetry::clearValues() {
    frame_fragments = "";

    for (auto& el : data) {
        el.second.values.clear();
    }

    times_unix.clear();
    times_elapsed.clear();

    set_start_time();
}

void BSP::Telemetry::clear() {
    frame_fragments = "";

    data.clear();
    times_unix.clear();
    times_elapsed.clear();

    set_start_time();
}

double BSP::Telemetry::get_unix_time() {
    auto now = std::chrono::system_clock::now();

    return std::chrono::duration<double>(now.time_since_epoch()).count();
}

void BSP::Telemetry::dump_data(std::string path, Limits limits, PlotTimeStyle ts) const {
    std::ofstream dump(path);

    const auto& times = (ts == DATETIME) ? times_unix : times_elapsed;

    // check if the file was opened correctly
    if (!dump.is_open()) {
        std::println(stderr, "Error while opening dump file.");
        return;
    }

    // === write labels ===
    std::print(dump, "times");

    std::lock_guard<std::mutex> lock(BSP::plot_mtx);
    for (const auto& el : data) {
        std::print(dump, ";{}", el.second.name);
    }
    std::println(dump);

    // get time window
    auto min_x_it = std::lower_bound(times.begin(), times.end(), limits.x_min);
    auto max_x_it = std::upper_bound(times.begin(), times.end(), limits.x_max);

    // check if found min is within the range
    if (min_x_it != times.end() && *min_x_it < limits.x_min || *min_x_it > limits.x_max) {
        min_x_it = times.end();
    }

    max_x_it = std::prev(max_x_it);

    // write data to file
    for (auto it = min_x_it; it <= max_x_it; it++) {
        std::print(dump, "{}", (ts == DATETIME) ? formatUnixTimestamp(*it) : std::format("{:.0f}", *it));

        for (const auto& el : data) {
            size_t data_i = std::distance(times.begin(), it);
            std::string el_str = "";

            if (data_i < el.second.values.size()) {
                auto val = el.second.values.at(data_i);
                
                if (val >= limits.y_min && val <= limits.y_max) {
                    el_str = std::to_string(el.second.values.at(data_i));
                }
            }

            std::print(dump, ";{}", el_str);     
        }
        std::println(dump);
    }
    std::println("Dump done!!");

    dump.close();
}

std::string BSP::Telemetry::formatUnixTimestamp(double unix_timestamp) {
    // get time from system clock
    time_t time = static_cast<time_t>(unix_timestamp);

    std::tm tm = *std::localtime(&time);

    // format the string
    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%d_%H:%M:%S");

    return oss.str();
}

bool BSP::Telemetry::is_empty() const {
    std::lock_guard<std::mutex> lock(plot_mtx);

    return data.empty();
}

