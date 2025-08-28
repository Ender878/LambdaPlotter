#ifndef __TELEMETRY_H__
#define __TELEMETRY_H__

#include "BSP/shared.h"
#include <chrono>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

namespace BSP {
    struct ChannelStyle;

    // struct containing all the attributes and values of plot channels
    typedef struct Channel {
        std::string         name;
        std::vector<double> values;
        std::vector<double> values_transformed;

        double              scale;
        double              prev_scale = 0.0;

        double              offset;
        double              prev_offset = 0.0;
    } Channel;

    // struct containing the frame format attributes
    typedef struct FrameFormat {
        char channel_sep[255] = " ";
        char frame_end[255]   = R"(\n)";
        char name_sep[255]    = ":";
        bool named            = false;
    } DataFormat;

    // Class responsible of all the operations performed on data read from the serial buffers
    class Telemetry {
        private:
            mutable std::mutex data_mtx;
            std::string frame_fragments;

            std::vector<double> times_unix;
            std::vector<double> times_elapsed;
            std::unordered_map<int, Channel> data;

            std::chrono::system_clock::time_point start_time;

            /**
             * @brief Correctly parse all the valid special chars (e.g. '\\n' -> '\n')
             * 
             * @param s
             * @return the modified string
             */
            static std::string format_special_chars(const char* s);
        public:
            DataFormat frame_format;

            /**
             * @brief Construct a new Telemetry object and initialize `start_time`
             * 
             */
            Telemetry();

            /**
             * @brief Parse the characters read from the serial buffer to valid data frames according to the frame format
             * 
             * @param  buffer char buffer of the read serial data
             * @return valid data frames if found, or an empty string if not
             */
            std::string parse_serial(std::vector<char>& buffer);

            /**
             * @brief Parse valid data frames to find and store values
             * 
             * @param frame_stream valid frames, coming from `parse_serial`
             */
            void parse_frame(const std::string&  frame_stream);

            /**
             * @brief Save data to a CSV file.
             * 
             * @param path       path to the file where the data will be saved
             * @param limits     plot limits where the data will be taken
             * @param ch_styles  plot channels style
             * @param ts         time format (DATETIME or ELAPSED) 
             */
            void dump_data(std::string path, Limits limits, std::unordered_map<int, ChannelStyle> ch_styles, PlotTimeStyle ts = ELAPSED) const;

            /**
             * @brief Get the actual unix time
             *
             */
            static double get_unix_time();

            /**
             * @brief Get the elapsed time from `start_time` in millis
             * 
             */
            int get_elapsed_time();

            /**
             * @brief Get the frame fragments
             * 
             */
            std::string get_frame_fragments() const { return frame_fragments; }

            /**
             * @brief reset the frame fragments string
             * 
             */
            void clear_fragments() { frame_fragments = ""; }

            void    set_start_time() { start_time = std::chrono::system_clock::now(); };
            double  get_start_time() const;

            inline std::unordered_map<int, Channel>* get_data()     { return &data; }
            inline std::vector<double>* get_unix_timestamps()       { return &times_unix; };
            inline std::vector<double>* get_elapsed_timestamps()    { return &times_elapsed; };

            static std::string format_datetime(double unix_timestamp);

            std::mutex& get_data_mtx();

            bool is_empty() const;
            void clear_values();
            void clear();
    };
}

#endif
