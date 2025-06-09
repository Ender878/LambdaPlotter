#ifndef __BSP_DEFINES_H__
#define __BSP_DEFINES_H__

#include <mutex>

#define WIN_WIDTH  900
#define WIN_HEIGHT 600

#define THREAD_READ_DELAY 5 // ms

#define PLOT_TIME_WINDOW 5000

namespace BSP {
    static std::mutex mtx;
}

#endif
