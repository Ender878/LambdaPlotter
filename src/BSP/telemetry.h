#ifndef __TELEMETRY_H__
#define __TELEMETRY_H__

#include "../common/shared.h"
#include <chrono>
#include <string>
#include <unordered_map>
#include <vector>

namespace BSP {
    class Telemetry {
        private:
            std::string incomplete_data_chunk;

            std::vector<double> times;
            std::unordered_map<int, std::vector<double>> values;
    
            size_t max_size;
            std::chrono::system_clock::time_point start_time;

            void dataFormatSpaces(const std::string& data_chunk);
            std::string getCurrDateTime() const;
        public:
            Telemetry(size_t t_max_size = DATA_MAX_SIZE);

            void processData(std::vector<char>& buffer);

            void clearIncompleteChunk() { incomplete_data_chunk = ""; }

            void setStartTime() { start_time = std::chrono::system_clock::now(); };
            double getStartTime() const;

            void clear();

            double getUnixTimestamp() const;

            inline std::unordered_map<int, std::vector<double>> const* getData() const { return &values; }
            inline std::vector<double> const* getTimestamps() const { return &times; };

            void dump_data(std::string device_path) const;
    };
}

#endif
