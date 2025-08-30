#ifndef __CONTROLLER_H__
#define __CONTROLLER_H__

#include "plotView.h"
#include "telemetry.h"
#include "toolbar.h"
#include <mutex>
#include <string>

namespace LP {
    class Controller {
        private:
            // reading thread's mutex
            static std::mutex thread_mtx;

            static ToolBar   toolbar;
            static Telemetry tel;
            static PlotView  plot_view;
            
            // application state variable (either READING or IDLE)
            static app_state_t prev_app_state;
            static app_state_t curr_app_state;

            /**
             * @brief Start to read from the serial port on another thread.
             * 
             * @param port // the serial port's file path
             * @param baud // baud rate used for reading from the port
             */
            static void start_serial_reading(std::string port, size_t baud);

            /**
             * @brief Wrapper method for saving the plot view to a csv file 
             * 
             */
            static void save_file();
        public:
            /**
             * @brief Method responsible to handle the application's state and all his functions
             * 
             */
            static void update();
            
            /**
             * @brief Makes shure that all the application's threads and operations are gracefully terminated before the process termination
             * 
             */
            static void shutdown();
    };
}

#endif
