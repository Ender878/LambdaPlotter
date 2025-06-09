#include "telemetry.h"

void BSP::Telemetry::addDataPoint(double time, double value) {
    values.push_back(value);
    times.push_back(time);

    if (times.size() > max_size) {
        values.pop_front();
        times.pop_front();
    }
}

void BSP::Telemetry::clear() {
    values.clear();
    times.clear();
}

double BSP::Telemetry::getRelativeTime() {
    auto now = std::chrono::steady_clock::now();
    auto duration = now - start_time;

    return std::chrono::duration<double>(duration).count();
}
