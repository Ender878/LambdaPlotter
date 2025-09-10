#ifndef __TOOLBAR_H__
#define __TOOLBAR_H__

#include "LP/shared.h"
#include <cstddef>
#include <optional>
#include <string>
#include <vector>

namespace LP {
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
              : combobox_port_index(std::nullopt), combobox_baud_index(6), combobox_time_index(2),
                open_close_button(false), refresh_button(false), save_button(false), clear_button(false) {}

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

            [[nodiscard]] inline bool getClearButton()                        const { return clear_button; }
            [[nodiscard]] inline bool getSaveButton()                         const { return save_button; }
            [[nodiscard]] inline size_t getComboboxTimeIndex()                const { return combobox_time_index; }
            [[nodiscard]] inline std::optional<size_t> getComboboxPortIndex() const { return combobox_port_index;}
            [[nodiscard]] inline size_t getComboboxBaudIndex()                const { return combobox_baud_index; }
            [[nodiscard]] inline bool getOpenCloseButton()                    const { return open_close_button; }
            [[nodiscard]] inline bool getRefreshButton()                      const { return refresh_button; }
            [[nodiscard]] inline std::string getCurrentPort()                 const { return current_port; }

            inline void setClearButton(const bool value)                          { clear_button = value; }
            inline void setComboboxTimeIndex(const size_t value)                  { combobox_time_index = value; }
            inline void setComboboxPortIndex(const std::optional<size_t> value)   { combobox_port_index = value; }
            inline void setComboboxBaudIndex(const size_t value)                  { combobox_baud_index = value; }
            inline void setOpenCloseButton(const bool value)                      { open_close_button = value; }
            inline void setRefreshButton(const bool value)                        { refresh_button = value; }
            inline void setCurrentPort(const char *value)                         { current_port = value; }
            inline void setSaveButton(const bool value)                           { save_button = value; }
    };
}

#endif
