#ifndef __TOOLBAR_H__
#define __TOOLBAR_H__

#include "BSP/shared.h"
#include <cstddef>
#include <optional>
#include <string>
#include <vector>

namespace BSP {
    class ToolBar {
        private:
            std::optional<size_t> combobox_port_index;
            std::string current_port;
            size_t combobox_baud_index;
            size_t combobox_time_index;
            
            bool open_close_button;
            bool refresh_button;
            bool save_button;
            bool clear_button;
        public:
            ToolBar()
                : combobox_port_index(std::nullopt), current_port(""), combobox_baud_index(6), combobox_time_index(2),
                open_close_button(false), refresh_button(false) {}

            /**
             * @brief Update the selected serial port based on the actual available ports
             * 
             * @param serial_ports vector containing the available serial ports
             */
            void update_serial_ports(const std::vector<std::string> &serial_ports);

            /**
             * @brief Render the serial configuration widgets
             * 
             * @param app_state     the current application state (either READING or IDLE)
             * @param no_telemetry  boolean used to check if there are already plotted values
             * @param serial_ports  array of available serial ports
             */
            void render(app_state_t app_state, bool no_telemetry, const std::vector<std::string>& serial_ports);

            /**
             * @brief Get the new app state
             * 
             * @param curr_app_state the current app state
             * @return the new app state
             */
            app_state_t get_new_app_state(const app_state_t &curr_app_state) const;

            inline bool getClearButton()                        const { return clear_button; }
            inline bool getSaveButton()                         const { return save_button; }
            inline size_t getComboboxTimeIndex()                const { return combobox_time_index; }
            inline std::optional<size_t> getComboboxPortIndex() const { return combobox_port_index;}
            inline size_t getComboboxBaudIndex()                const { return combobox_baud_index; }
            inline bool getOpenCloseButton()                    const { return open_close_button; }
            inline bool getRefreshButton()                      const { return refresh_button; }
            inline std::string getCurrentPort()                 const { return current_port; }

            inline void setClearButton(bool value)                          { clear_button = value; }
            inline void setComboboxTimeIndex(size_t value)                  { combobox_time_index = value; }
            inline void setComboboxPortIndex(std::optional<size_t> value)   { combobox_port_index = value; }
            inline void setComboboxBaudIndex(size_t value)                  { combobox_baud_index = value; }
            inline void setOpenCloseButton(bool value)                      { open_close_button = value; }
            inline void setRefreshButton(bool value)                        { refresh_button = value; }
            inline void setCurrentPort(const char *value)                   { current_port = value; }
            inline void setSaveButton(bool value)                           { save_button = value; } 
    };
}

#endif
