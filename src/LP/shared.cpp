#include <LP/shared.h>
#include <array>

#ifndef _WIN32
#include <termios.h>
#endif

namespace LP {
const std::array<combobox_tuple_t, BAUD_RATES_SIZE> baud_rates = {
#ifndef _WIN32
    {{"300", B300},
                  {"600", B600},
                  {"1200", B1200},
                  {"1800", B1800},
                  {"2400", B2400},
                  {"4800", B4800},
                  {"9600", B9600},
                  {"19200", B19200},
                  {"38400", B38400},
                  {"57600", B57600},
                  {"115200", B115200}}
#else
    {{"300", 300},
     {"600", 600},
     {"1200", 1200},
     {"1800", 1800},
     {"2400", 2400},
     {"4800", 4800},
     {"9600", 9600},
     {"19200", 19200},
     {"38400", 38400},
     {"57600", 57600},
     {"115200", 115200}}
#endif
};

const std::array<combobox_tuple_t, TIME_WINDOWS_SIZE> time_windows = {
    {{"1 s", 1},
     {"5 s", 5},
     {"10 s", 10},
     {"30 s", 30},
     {"1 min", 60},
     {"5 min", 60 * 5},
     {"10 min", 60 * 10},
     {"30 min", 60 * 30},
     {"None", 0}}
};
} // namespace LP
