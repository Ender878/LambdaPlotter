#include <LP/shared.h>
#include <LP/telemetry.h>
#include <algorithm>
#include <chrono>
#include <cmath>
#include <ctime>
#include <format>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <mutex>
#include <ostream>
#include <ranges>
#include <regex>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

#include "LP/plotView.h"

LP::Telemetry::Telemetry() : start_time(std::chrono::system_clock::now())
{
}

std::string LP::Telemetry::parse_serial(std::vector<char>& buffer)
{
    // char buffer to valid string
    buffer.push_back('\0');
    std::string buffer_str = buffer.data();

    // valid frames found from the read serial buffer
    std::string frame_stream;

    // last frame end token index
    size_t last_frame_end_i = 0;

    // apply special characters inserted by the user
    const std::string frame_end     = format_special_chars(frame_format.frame_end);
    const size_t      frame_end_len = frame_end.length();

    do
    {
        // search for the frame end token
        const size_t frame_end_i = buffer_str.find(frame_end, last_frame_end_i);

        // if it has been found, there is a valid frame. Update frame_stream.
        // if there is no token, append the characters to the fragment string
        if (frame_end_i != std::string::npos)
        {
            frame_stream +=
                frame_fragments + buffer_str.substr(last_frame_end_i, frame_end_i - last_frame_end_i + frame_end_len);
            frame_fragments = "";
        }
        else
        {
            frame_fragments += buffer_str.substr(last_frame_end_i, buffer_str.length());
        }

        last_frame_end_i = (frame_end_i == std::string::npos) ? frame_end_i : frame_end_i + frame_end_len;
    } while (last_frame_end_i != std::string::npos);

    return frame_stream;
}

void LP::Telemetry::parse_frame(const std::string& frame_stream)
{
    auto it_end = std::sregex_iterator();

    std::string frame_end   = format_special_chars(frame_format.frame_end);
    std::string channel_sep = format_special_chars(frame_format.channel_sep);
    std::string name_sep    = format_special_chars(frame_format.name_sep);

    // regex to find single frames
    std::string frame_pattern = std::format("(.+{})", frame_end);
    std::string value_pattern;

    if (frame_format.named)
    {
        value_pattern = std::format(R"((.+?){}(-?\d+\.?\d*)(?:{}|{}))", name_sep, channel_sep, frame_end);
    }
    else
    {
        value_pattern = std::format(R"((-?\d+\.?\d*)(?:{}|{}))", channel_sep, frame_end);
    }

    // regexes for finding frames and values
    std::regex frame_rgx(frame_pattern);
    std::regex value_rgx(value_pattern);

    // iterate through all the valid frames that matched the regex
    auto frames_it_begin = std::sregex_iterator(frame_stream.begin(), frame_stream.end(), frame_rgx);
    for (auto it_f = frames_it_begin; it_f != it_end; ++it_f)
    {
        std::string frame = it_f->str();

        int ch_id = 1;

        // iterate through all the valid values found in the frame that matched the regex
        auto values_it_begin = std::sregex_iterator(frame.begin(), frame.end(), value_rgx);
        for (auto it_v = values_it_begin; it_v != it_end; ++it_v)
        {
            std::string name = frame_format.named ? (*it_v)[1].str() : std::format("Data {}", ch_id);

            std::string value = (*it_v)[frame_format.named ? 2 : 1].str();

            // if the channel doesn't exist, initialize it
            if (!data.contains(ch_id))
            {
                data[ch_id].name   = name;
                data[ch_id].scale  = 1.0f;
                data[ch_id].offset = 0.0f;
            }

            // save the value
            try
            {
                const double val = std::stod(value, nullptr);

                data[ch_id].values.push_back(val);
            }
            catch ([[maybe_unused]] const std::invalid_argument& e)
            {
                data[ch_id].values.push_back(NAN);
            }

            ch_id++;
        }

        // set the time point only if something has been read
        if (ch_id > 1)
        {
            times_unix.push_back(get_unix_time());
            times_elapsed.push_back(get_elapsed_time());
        }
    }
}

std::string LP::Telemetry::format_special_chars(const char* s)
{
    std::string result = s;

    result = std::regex_replace(result, std::regex(R"(\\n)"), "\n");
    result = std::regex_replace(result, std::regex(R"(\\r)"), "\r");
    result = std::regex_replace(result, std::regex(R"(\\t)"), "\t");
    result = std::regex_replace(result, std::regex(R"(\\f)"), "\f");
    result = std::regex_replace(result, std::regex(R"(\\v)"), "\v");

    return result;
}

int LP::Telemetry::get_elapsed_time() const
{
    const double now   = get_unix_time();
    const double start = get_start_time();

    return static_cast<int>((now - start) * 1000);
}

double LP::Telemetry::get_start_time() const
{
    return std::chrono::duration<double>(start_time.time_since_epoch()).count();
}

// remove
void LP::Telemetry::clear_values()
{
    frame_fragments = "";

    for (auto& val : data | std::views::values)
    {
        val.values.clear();
    }

    times_unix.clear();
    times_elapsed.clear();

    set_start_time();
}

void LP::Telemetry::clear(const bool clear_fragments)
{
    if (clear_fragments)
        frame_fragments = "";

    data.clear();
    times_unix.clear();
    times_elapsed.clear();
}

double LP::Telemetry::get_unix_time()
{
    const auto now = std::chrono::system_clock::now();

    return std::chrono::duration<double>(now.time_since_epoch()).count();
}

void LP::Telemetry::dump_data(const std::string&                    path,
                              Limits                                limits,
                              std::unordered_map<int, ChannelStyle> ch_styles,
                              PlotTimeStyle                         ts) const
{
    std::ofstream dump(path);

    const auto& times = (ts == DATETIME) ? times_unix : times_elapsed;

    // check if the file was opened correctly
    if (!dump.is_open())
    {
        std::cerr << "Error while opening dump file." << std::endl;
        return;
    }

    // === write labels ===
    dump << "times";

    std::lock_guard lock(data_mtx);
    for (const auto& [id, channel] : data)
    {
        if (ch_styles[id].show)
            dump << ";" << channel.name;
    }
    dump << "\n";

    // get time window
    auto min_x_it = std::ranges::lower_bound(times, limits.x_min);
    auto max_x_it = std::ranges::upper_bound(times, limits.x_max);

    // check if found min is within the range
    if ((min_x_it != times.end()) && (*min_x_it < limits.x_min || *min_x_it > limits.x_max))
    {
        min_x_it = times.end();
    }

    max_x_it = std::prev(max_x_it);

    // write data to file
    for (auto it = min_x_it; it <= max_x_it; ++it)
    {
        dump << ((ts == DATETIME) ? format_datetime(*it) : std::format("{:.0f}", *it));

        for (const auto& [id, channel] : data)
        {
            if (!ch_styles[id].show)
                continue;

            size_t      data_i = std::distance(times.begin(), it);
            std::string val_str;

            if (data_i < channel.values.size())
            {

                if (double val = channel.values.at(data_i) * channel.scale + channel.offset;
                    val >= limits.y_min && val <= limits.y_max)
                {
                    val_str = std::to_string(val);

                    std::ranges::replace(val_str, '.', ',');
                }
            }

            dump << ";" << val_str;
        }
        dump << "\n";
    }
    std::cout << "Dump done!!" << std::endl;

    dump.close();
}

std::string LP::Telemetry::format_datetime(const double unix_timestamp)
{
    // get time from system clock
    const auto time = static_cast<time_t>(unix_timestamp);

    const std::tm tm = *std::localtime(&time);

    // format the string
    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%d_%H:%M:%S");

    return oss.str();
}

bool LP::Telemetry::is_empty() const
{
    std::lock_guard lock(data_mtx);

    return data.empty();
}

std::mutex& LP::Telemetry::get_data_mtx() const
{
    return data_mtx;
}
