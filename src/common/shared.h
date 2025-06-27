#ifndef __BSP_SHARED_H__
#define __BSP_SHARED_H__

#include <mutex>
#include <termios.h>

#define WIN_WIDTH  900
#define WIN_HEIGHT 600

#define THREAD_READ_DELAY 5 // ms
#define LOG_PATH "/Dev/better_serial_plotter"

#define DATA_MAX_SIZE 100000

namespace BSP {
    extern std::mutex plot_mtx;
    extern std::mutex thread_mtx;

    typedef struct baud_rate_t {
        const char* str;
        const int   value;
    } baud_rate_t;

    typedef enum app_state_t {
        READING,
        IDLE,
    } app_state_t;

    extern const baud_rate_t baud_rates[];

    extern const size_t baud_rates_size;
}

#endif
