#pragma once

#include "ILayer.hpp"

namespace LP {
    typedef enum protocol_tab {
        SERIAL,
        UDP,
    } protocol_tab;

    class Toolbar : public ILayer {
    private:
        protocol_tab active_protocol_tab;

        void render_serial();
        void render_udp();
    public:
        Toolbar() : active_protocol_tab(SERIAL) {}

        void on_update(double dt) override;
        void on_render() override;
    };
}
