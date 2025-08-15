#ifndef __TELEMETRY_H__
#define __TELEMETRY_H__

#include <common/shared.h>
#include <chrono>
#include <string>
#include <unordered_map>
#include <vector>

namespace BSP {
    typedef struct PlotData {
        std::string         name;
        std::vector<double> values;
        // to apply to the entire array of values
        double              scale;
        double              offset;
    } PlotData;

    typedef struct DataFormat {
        char channel_sep[255] = " ";
        char frame_end[255]   = "\\n";
        char name_sep[255]    = ":";
        bool named            = false;
    } DataFormat;

    class Telemetry {
        private:
            std::string frame_fragments;

            std::vector<double> times_unix;
            std::vector<double> times_elapsed;
            std::unordered_map<int, PlotData> data;
    
            size_t max_size;
            std::chrono::system_clock::time_point start_time;

            void dataFormatSpaces(const std::string& data_chunk);
        public:
            DataFormat frame_format;

            Telemetry(size_t t_max_size = DATA_MAX_SIZE);

            void processData(std::vector<char>& buffer);

            std::string parse_serial(std::vector<char>& buffer);
            void        parse_frame(const std::string&  frame_stream);

            static double get_unix_time();
            int           get_elapsed_time();

            std::string get_frame_fragments() const { return frame_fragments; }

            void clearIncompleteChunk() { frame_fragments = ""; }

            void    set_start_time()      { start_time = std::chrono::system_clock::now(); };
            double  get_start_time() const;

            inline std::unordered_map<int, PlotData>* getData()                { return &data; }
            inline std::vector<double> const* get_unix_timestamps()    const   { return &times_unix; };
            inline std::vector<double> const* get_elapsed_timestamps() const   { return &times_elapsed; };

            static std::string formatUnixTimestamp(double unix_timestamp);

            static std::string format_special_chars(const char* s);

            void dump_data(std::string path) const;

            bool is_empty() const;
            void clearValues();
            void clear();
    };
}

#endif
