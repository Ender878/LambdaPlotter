#include "shared.h"

namespace BSP {
    std::mutex plot_mtx;
    std::mutex thread_mtx;

    const combobox_tuple_t baud_rates[] = {
        {"300",     B300},
        {"600",     B600},
        {"1200",    B1200},
        {"1800",    B1800},
        {"2400",    B2400},
        {"4800",    B4800},
        {"9600",    B9600},
        {"19200",   B19200},
        {"38400",   B38400},
        {"57600",   B57600},
        {"115200",  B115200}
    };

    const combobox_tuple_t time_windows[] = {
        {"1 s",      1},
        {"5 s",      5},
        {"10 s",     10},
        {"30 s",     30},
        {"1 min",    60},
        {"5 min",    60 * 5},
        {"10 min",   60 * 10},
        {"30 min",   60 * 30},
        {"None",     0}
    };

    constexpr size_t baud_rates_size = sizeof(baud_rates) / sizeof(*baud_rates);
    constexpr size_t baud_time_size  = sizeof(time_windows) / sizeof(*time_windows);
}
