#ifndef __BSP_SHARED_H__
#define __BSP_SHARED_H__

#include <array>
#include <termios.h>

#define MIN_WIN_WIDTH           1375
#define MIN_WIN_HEIGHT          700

#define THREAD_READ_DELAY       5

#define DATA_MAX_SIZE           100000

#define BAUD_RATES_SIZE         11
#define TIME_WINDOWS_SIZE       9 

namespace BSP {
    typedef struct combobox_tuple_t {
        const char* str;
        const int   value;
    } combobox_tuple_t;

    typedef struct Limits {
        double x_min;
        double x_max;

        double y_min;
        double y_max;
    } Limits;

    typedef enum app_state_t {
        READING,
        IDLE,
    } app_state_t;

    typedef enum PlotTimeStyle {
        DATETIME,
        ELAPSED
    } PlotTimeStyle;

    extern const std::array<combobox_tuple_t, BAUD_RATES_SIZE>    baud_rates;
    extern const std::array<combobox_tuple_t, TIME_WINDOWS_SIZE>  time_windows;
}

#endif
