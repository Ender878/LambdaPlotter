#include "LP/plotView.h"
#include <LP/shared.h>
#include <LP/telemetry.h>
#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <format>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <mutex>
#include <ostream>
#include <regex>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

LP::Telemetry::Telemetry() : frame_fragments(""), start_time(std::chrono::system_clock::now()) {}

std::string LP::Telemetry::parse_serial(std::vector<char> &buffer) {
    // char buffer to valid string
    buffer.push_back('\0');
    std::string buffer_str = buffer.data();

    // valid frames found from the read serial buffer
    std::string frame_stream = "";

    // last frame end token index
    size_t last_frame_end_i = 0;

    // apply special characters inserted by the user
    std::string frame_end     = Telemetry::format_special_chars(frame_format.frame_end);
    size_t      frame_end_len = frame_end.length();

    do {
        // search for the frame end token
        size_t frame_end_i = buffer_str.find(frame_end, last_frame_end_i);

        // if it has been found, there is a valid frame. Update frame_stream.
        // if there is no token, append the characters to the fragment string
        if (frame_end_i != std::string::npos) {
            frame_stream +=
                frame_fragments +
                buffer_str.substr(last_frame_end_i, frame_end_i - last_frame_end_i + frame_end_len);
            frame_fragments = "";
        } else {
            frame_fragments += buffer_str.substr(last_frame_end_i, buffer_str.length());
        }

        last_frame_end_i =
            (frame_end_i == std::string::npos) ? frame_end_i : frame_end_i + frame_end_len;
    } while (last_frame_end_i != std::string::npos);

    return frame_stream;
}

void LP::Telemetry::parse_frame(const std::string &frame_stream) {
    auto it_end = std::sregex_iterator();

    std::string frame_end   = Telemetry::format_special_chars(frame_format.frame_end);
    std::string channel_sep = Telemetry::format_special_chars(frame_format.channel_sep);
    std::string name_sep    = Telemetry::format_special_chars(frame_format.name_sep);

    // regex to find single frames
    std::string frame_pattern = std::format("(.+{})", frame_end);
    std::string value_pattern;

    if (frame_format.named) {
        value_pattern = std::format(R"((.+?){}(.*?)(?:{}|{}))", name_sep, channel_sep, frame_end);
    } else {
        value_pattern = std::format(R"((.+?)(?:{}|{}))", channel_sep, frame_end);
    }

    // regexes for finding frames and values
    std::regex frame_rgx(frame_pattern);
    std::regex value_rgx(value_pattern);

    // iterate throught all the valid frames that matched the regex
    auto frames_it_begin =
        std::sregex_iterator(frame_stream.begin(), frame_stream.end(), frame_rgx);
    for (auto it_f = frames_it_begin; it_f != it_end; ++it_f) {
        std::string frame = it_f->str();

        size_t ch_id = 1;

        // iterate throught all the valid values found in the frame that matched the regex
        auto values_it_begin = std::sregex_iterator(frame.begin(), frame.end(), value_rgx);
        for (auto it_v = values_it_begin; it_v != it_end; ++it_v) {
            std::string name =
                frame_format.named ? (*it_v)[1].str() : std::format("Data {}", ch_id);

            std::string value = (*it_v)[frame_format.named ? 2 : 1].str();

            // if the channel doesn't exist, initialize it
            if (!data.contains(ch_id)) {
                data[ch_id].name   = name;
                data[ch_id].scale  = 1.0f;
                data[ch_id].offset = 0.0f;
            }

            // save the value
            try {
                data[ch_id].values.push_back(std::stod(value));
                data[ch_id].values_transformed.push_back(std::stod(value) * data[ch_id].scale +
                                                         data[ch_id].offset);
            } catch (const std::invalid_argument &e) {
                data[ch_id].values.push_back(NAN);
            }

            ch_id++;
        }

        // set the time point only if something has been read
        if (ch_id > 1) {
            times_unix.push_back(get_unix_time());
            times_elapsed.push_back(get_elapsed_time());
        }
    }
}

std::string LP::Telemetry::format_special_chars(const char *s) {
    std::string result = s;

    result = std::regex_replace(result, std::regex(R"(\\n)"), "\n");
    result = std::regex_replace(result, std::regex(R"(\\r)"), "\r");
    result = std::regex_replace(result, std::regex(R"(\\t)"), "\t");
    result = std::regex_replace(result, std::regex(R"(\\f)"), "\f");
    result = std::regex_replace(result, std::regex(R"(\\v)"), "\v");

    return result;
}

int LP::Telemetry::get_elapsed_time() {
    double now        = get_unix_time();
    double start_time = get_start_time();

    return int((now - start_time) * 1000);
}

double LP::Telemetry::get_start_time() const {
    return std::chrono::duration<double>(start_time.time_since_epoch()).count();
}

void LP::Telemetry::clear_values() {
    frame_fragments = "";

    for (auto &el : data) {
        el.second.values.clear();
    }

    times_unix.clear();
    times_elapsed.clear();

    set_start_time();
}

void LP::Telemetry::clear() {
    frame_fragments = "";

    data.clear();
    times_unix.clear();
    times_elapsed.clear();

    set_start_time();
}

double LP::Telemetry::get_unix_time() {
    auto now = std::chrono::system_clock::now();

    return std::chrono::duration<double>(now.time_since_epoch()).count();
}

void LP::Telemetry::dump_data(std::string path, Limits limits,
                              std::unordered_map<int, ChannelStyle> ch_styles,
                              PlotTimeStyle                         ts) const {
    std::ofstream dump(path);

    const auto &times = (ts == DATETIME) ? times_unix : times_elapsed;

    // check if the file was opened correctly
    if (!dump.is_open()) {
        std::cerr << "Error while opening dump file." << std::endl;
        return;
    }

    // === write labels ===
    dump << "times";

    std::lock_guard<std::mutex> lock(data_mtx);
    for (const auto &el : data) {
        if (ch_styles[el.first].show) dump << ";" << el.second.name;
    }
    dump << "\n";

    // get time window
    auto min_x_it = std::lower_bound(times.begin(), times.end(), limits.x_min);
    auto max_x_it = std::upper_bound(times.begin(), times.end(), limits.x_max);

    // check if found min is within the range
    if ((min_x_it != times.end()) && (*min_x_it < limits.x_min || *min_x_it > limits.x_max)) {
        min_x_it = times.end();
    }

    max_x_it = std::prev(max_x_it);

    // write data to file
    for (auto it = min_x_it; it <= max_x_it; it++) {
        dump << ((ts == DATETIME) ? format_datetime(*it) : std::format("{:.0f}", *it));

        for (const auto &el : data) {
            if (!ch_styles[el.first].show) continue;

            size_t      data_i  = std::distance(times.begin(), it);
            std::string val_str = "";

            if (data_i < el.second.values_transformed.size()) {
                double val = el.second.values_transformed.at(data_i);

                if (val >= limits.y_min && val <= limits.y_max) {
                    val_str = std::to_string(val);

                    std::replace(val_str.begin(), val_str.end(), '.', ',');
                }
            }

            dump << ";" << val_str;
        }
        dump << "\n";
    }
    std::cout << "Dump done!!" << std::endl;

    dump.close();
}

std::string LP::Telemetry::format_datetime(double unix_timestamp) {
    // get time from system clock
    time_t time = static_cast<time_t>(unix_timestamp);

    std::tm tm = *std::localtime(&time);

    // format the string
    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%d_%H:%M:%S");

    return oss.str();
}

bool LP::Telemetry::is_empty() const {
    std::lock_guard<std::mutex> lock(data_mtx);

    return data.empty();
}

std::mutex &LP::Telemetry::get_data_mtx() { return data_mtx; }
