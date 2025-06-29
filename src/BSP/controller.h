#ifndef __CONTROLLER_H__
#define __CONTROLLER_H__

#include "telemetry.h"
#include "toolbar.h"
#include <string>

namespace BSP {
    class Controller {
        private:
            static ToolBar toolbar;
            static Telemetry tel;
            
            static app_state_t prev_app_state;
            static app_state_t curr_app_state;

            static void startSerialReading(std::string port, size_t baud);
            static void saveFile();
        public:
            static void update();
            static void shutdown();
    };
}

#endif
