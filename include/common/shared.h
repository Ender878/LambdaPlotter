#ifndef __BSP_SHARED_H__
#define __BSP_SHARED_H__

#include <mutex>
#include <termios.h>

#define MIN_WIN_WIDTH  1400
#define MIN_WIN_HEIGHT 800

#define THREAD_READ_DELAY 5 // ms

#define DATA_MAX_SIZE 100000

namespace BSP {
    extern std::mutex plot_mtx;
    extern std::mutex thread_mtx;

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

    extern const combobox_tuple_t baud_rates[];
    extern const combobox_tuple_t time_windows[];

    extern const size_t baud_rates_size;
    extern const size_t time_windows_size;
}

#endif
