#include "shared.h"

namespace BSP {
    std::mutex plot_mtx;
    std::mutex thread_mtx;

    const baud_rate_t baud_rates[] = {
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

    constexpr size_t baud_rates_size = sizeof(baud_rates) / sizeof(*baud_rates);
}
