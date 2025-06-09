#ifndef __TELEMETRY_H__
#define __TELEMETRY_H__

#include <chrono>
#include <deque>

namespace BSP {
    class Telemetry {
        private:
            std::deque<double> times;
            std::deque<double> values;
    
            size_t max_size;
            std::chrono::steady_clock::time_point start_time;
        public:
            Telemetry(size_t t_max_size = 10000)
                : start_time(std::chrono::steady_clock::now()), max_size(t_max_size) {}
    
            void addDataPoint(double time, double value);
            void clear();

            double getRelativeTime();
    };
}

#endif
