#ifndef __TELEMETRY_H__
#define __TELEMETRY_H__

#include <common/shared.h>
#include <chrono>
#include <string>
#include <unordered_map>
#include <vector>

namespace BSP {
    struct ChannelStyle;

    typedef struct Channel {
        std::string         name;
        std::vector<double> values;
        std::vector<double> values_transformed;

        double              scale;
        double              prev_scale = 0.0;

        double              offset;
        double              prev_offset = 0.0;
    } Channel;

    typedef struct DataFormat {
        char channel_sep[255] = " ";
        char frame_end[255]   = R"(\n)";
        char name_sep[255]    = ":";
        bool named            = false;
    } DataFormat;

    class Telemetry {
        private:
            std::string frame_fragments;

            std::vector<double> times_unix;
            std::vector<double> times_elapsed;
            std::unordered_map<int, Channel> data;
    
            size_t max_size;
            std::chrono::system_clock::time_point start_time;

            void dataFormatSpaces(const std::string& data_chunk);
        public:
            DataFormat frame_format;

            Telemetry(size_t t_max_size = DATA_MAX_SIZE);

            std::string parse_serial(std::vector<char>& buffer);
            void        parse_frame(const std::string&  frame_stream);
            bool        validate_frame(const std::string& frame_stream);

            static double get_unix_time();
            int           get_elapsed_time();

            std::string get_frame_fragments() const { return frame_fragments; }

            void clearIncompleteChunk() { frame_fragments = ""; }

            void    set_start_time() { start_time = std::chrono::system_clock::now(); };
            double  get_start_time() const;

            inline std::unordered_map<int, Channel>* getData()     { return &data; }
            inline std::vector<double>* get_unix_timestamps()      { return &times_unix; };
            inline std::vector<double>* get_elapsed_timestamps()   { return &times_elapsed; };

            static std::string formatUnixTimestamp(double unix_timestamp);

            static std::string format_special_chars(const char* s);

            void dump_data(std::string path, Limits limits, std::unordered_map<int, ChannelStyle> ch_styles, PlotTimeStyle ts = ELAPSED) const;

            bool is_empty() const;
            void clearValues();
            void clear();
    };
}

#endif
