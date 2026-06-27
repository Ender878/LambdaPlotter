#pragma once

#include <mutex>
#include "ImApp.hpp"

namespace LP {
    typedef enum app_state {
        IDLE,
        RUNNING
    } app_state;

    class LambdaApp : public Core::ImApp {
        private:
            std::mutex m_state_mtx;
            app_state m_state;
        public:
            explicit LambdaApp(Core::app_specs specs = Core::app_specs());

            app_state get_app_state();
            void set_app_state(app_state t_state);

            std::mutex &get_state_mtx();

            [[nodiscard]] static LambdaApp& get();
    };
}

