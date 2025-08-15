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
            frame_stream += frame_fragments + buffer_str.substr(last_frame_end_i, frame_end_i - last_frame_end_i + frame_end_len);
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

// void BSP::Telemetry::processData(std::vector<char>& buffer) {
//     // filtered data chunks to be processed and plotted
//     std::string data_frame = "";

//     // add '\0` to the end of the buffer to make it a valid string
//     buffer.push_back('\0');
//     std::string buf_str = buffer.data();

//     // iter every character received from the buffer and for every '\n'
//     // encountered, add all the characters before it to the `plot_data` string,
//     // which will be processed and casted to be ready to be plotted.
//     for (const auto &c : buf_str) {
//         frame_fragments += c;

//         if (c == '\n') {
//             data_frame += frame_fragments;
//             frame_fragments = "";
//         }
//     }

//     if (!data_frame.empty()) {
//         std::stringstream data_frame_stream(data_frame);
//         std::string data_chunk;

//         // std::println("{}", data_frame);

//         // lock the thread while the plot data is updated
//         std::lock_guard<std::mutex> lock(BSP::plot_mtx);
//         while (std::getline(data_frame_stream, data_chunk, '\n')) {
//             dataFormatSpaces(data_chunk);

//             // if (times.size() > max_size) {

//             //     for (auto& stream : data) {
//             //         stream.second.values.erase(stream.second.values.begin());
//             //     }

//             //     times.erase(times.begin());
//             // }
//         }
//     }
// }

// void BSP::Telemetry::dataFormatSpaces(const std::string& data_chunk) {
//     std::stringstream data_chunk_stream(data_chunk);
//     std::string value;
//     size_t i = 1;

//     while (std::getline(data_chunk_stream, value, ' ')) {
//         if (!data.contains(i)) {
//             data[i].name   = std::format("Data {}", i);
//             data[i].scale  = 1.0f;
//             data[i].offset = 0.0f;
//         }

//         try {
//             data[i].values.push_back(std::stod(value) * data[i].scale + data[i].offset);
//         } catch (const std::exception &e) {
//             data[i].values.push_back(NAN);
//         }

//         i++;
//     }

//     // set the time point only if something has been read
//     if (i > 1) {
//         times.push_back(getUnixTimestamp());
//     }
// }

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
}

void BSP::Telemetry::clear() {
    frame_fragments = "";
    data.clear();
    times_unix.clear();
    times_elapsed.clear();
}

double BSP::Telemetry::get_unix_time() {
    auto now = std::chrono::system_clock::now();

    return std::chrono::duration<double>(now.time_since_epoch()).count();
}

// TODO: save only time window.
void BSP::Telemetry::dump_data(std::string path) const {
    // size_t i = 0;
    // std::ofstream dump(path);

    // if (!dump.is_open()) {
    //     std::println(stderr, "Error while opening dump file.");
    //     return;
    // }

    // std::print(dump, "times");

    // std::lock_guard<std::mutex> lock(BSP::plot_mtx);
    // for (const auto& el : data) {
    //     std::print(dump, ";{}", el.second.name);
    // }
    // std::println(dump);

    // for (const auto& t : times) {
    //     std::print(dump, "{}", formatUnixTimestamp(t));

    //     for (const auto& el : data) {
    //         std::string el_str = "";

    //         if (i <= times.size() - 1 && i <= el.second.values.size() - 1) {
    //             el_str = std::format("{}", el.second.values.at(i));
    //         }

    //         std::print(dump, ";{}", el_str);
    //     }
    //     std::println(dump);
    //     i++;
    // }

    // std::println("Dump done!!");

    // dump.close();
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

    return data.empty();
}

