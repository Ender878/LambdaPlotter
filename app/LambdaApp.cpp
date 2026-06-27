#include "LambdaApp.hpp"
#include <mutex>
#include "ImApp.hpp"

namespace LP {
    static LambdaApp *s_app;

    LambdaApp::LambdaApp(Core::app_specs specs) : Core::ImApp(std::move(specs)) {
        m_state = IDLE;
        s_app = this;
    }

    app_state LambdaApp::get_app_state() {
        std::lock_guard lock(m_state_mtx);
        return m_state;
    }

    void LambdaApp::set_app_state(app_state state) {
        std::lock_guard lock(m_state_mtx);
        m_state = state;
    }

    std::mutex &LambdaApp::get_state_mtx() {
        return m_state_mtx;
    }

    LambdaApp &LambdaApp::get() {
        if (!s_app) new LambdaApp;
        return *s_app;
    }
}
