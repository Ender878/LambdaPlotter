#ifndef __TOOLBAR_H__
#define __TOOLBAR_H__

#include "../common/shared.h"
#include <cstddef>
#include <optional>
#include <string>
#include <vector>

namespace BSP {

class ToolBar {
    private:
        // the following boolean flags are used to track the state of the
        // open/close button, the state of the serial reading operations and 
        // wether the list of available ports should be refreshed.
        std::optional<size_t> combobox_port_index;
        size_t combobox_baud_index;
        std::string current_port;
        bool open_close_button;
        bool refresh_button;

    public:
        ToolBar()
            : combobox_port_index(std::nullopt), combobox_baud_index(0),
            open_close_button(false), refresh_button(false), current_port("") {}

        void updateSerialPorts(const std::vector<std::string> &serial_ports);

        app_state_t getAppState(const app_state_t &curr_app_state) const;

        inline std::optional<size_t> getComboboxPortIndex() const { return combobox_port_index;}
        inline size_t getComboboxBaudIndex() const { return combobox_baud_index; }
        inline bool getOpenCloseButton() const { return open_close_button; }
        inline bool getRefreshButton() const { return refresh_button; }
        inline std::string getCurrentPort() const { return current_port; }

        inline void setComboboxPortIndex(std::optional<size_t> value) { combobox_port_index = value; }
        inline void setComboboxBaudIndex(size_t value) { combobox_baud_index = value; }
        inline void setOpenCloseButton(bool value) { open_close_button = value; }
        inline void setRefreshButton(bool value) { refresh_button = value; }
        inline void setCurrentPort(const char *value) { current_port = value; }
    };
} // namespace BSP

#endif
