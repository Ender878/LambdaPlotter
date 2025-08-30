#include <LP/toolbar.h>
#include <LP/serial.h>
#include "../fonts/lucide.h"
#include "LP/shared.h"

#include <imgui.h>
#include <algorithm>
#include <string>
#include <vector>

void LP::ToolBar::update_serial_ports(const std::vector<std::string> &serial_ports) {
    if (refresh_button) {
        // if the port list is empty, but the list index has a value, set the
        // index to null
        if (serial_ports.empty() && combobox_port_index.has_value()) {
            combobox_port_index = std::nullopt;
        } else if (!serial_ports.empty() && combobox_port_index.has_value()) {
            // if the port list is not empty and the list index is set, check if
            // the pointed value exists in the list and reassing the correct
            // index if the pointed element no longer exist, set the index to
            // null
            if (std::any_of(serial_ports.begin(), serial_ports.end(), [this](std::string v) { return v == current_port; })) {
                combobox_port_index = std::distance(serial_ports.begin(), std::find(serial_ports.begin(), serial_ports.end(), current_port));
            } else {
                combobox_port_index = std::nullopt;
            }
        }
    }

    current_port = combobox_port_index.has_value() ? serial_ports[combobox_port_index.value()] : "";
}

LP::app_state_t LP::ToolBar::get_new_app_state(const app_state_t &curr_app_state) const {
    app_state_t state = curr_app_state;

    if (open_close_button) {
        // if we are not already reading and there is a port selected, start
        // reading. if we are reading and the connection button has 
        // been pressed, stop reading.
        if (combobox_port_index.has_value() && curr_app_state == IDLE) {
            state = READING;
        } else if (curr_app_state == READING) {
            state = IDLE;
        }
    }

    return state;
}

void LP::ToolBar::render(app_state_t app_state, bool no_telemetry, const std::vector<std::string>& serial_ports) {
    // get prev selected port and baud rate
    const char* selected_port = combobox_port_index.has_value() && combobox_port_index <= serial_ports.size() 
        ? serial_ports[combobox_port_index.value()].c_str() 
        : "";
    const char* selected_baud = LP::baud_rates[combobox_baud_index].str;

    ImGui::SeparatorText("Serial interface");

    if (ImGui::BeginTable("##toolbar_layout", 2)) {
        ImGui::TableSetupColumn("Left",  ImGuiTableColumnFlags_WidthStretch, 0.4f);
        ImGui::TableSetupColumn("Right", ImGuiTableColumnFlags_WidthStretch, 0.6f);

        ImGui::TableNextRow();
        ImGui::TableNextColumn();

        // ====== Serial ports combo box ====== 
        ImGui::Text("Serial port");
        ImGui::TableNextColumn();
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - 5);
        if (ImGui::BeginCombo("##serial", selected_port)) {
            for (size_t i = 0; i < serial_ports.size(); i++) {
                bool is_selected = combobox_port_index == i;

                if (ImGui::Selectable(serial_ports[i].c_str(), is_selected)) {
                    combobox_port_index = i;
                    ImGui::SetItemDefaultFocus();
                }

            }
            ImGui::EndCombo();
        }
        
        ImGui::TableNextRow();
        ImGui::TableNextColumn();

        // ====== Baud rates combo box ======
        ImGui::Text("Baud rate");
        ImGui::TableNextColumn();
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - 5);
        if (ImGui::BeginCombo("##baud", selected_baud)) {
            for (size_t i = 0; i < LP::baud_rates.size(); i++) {
                bool selected = combobox_baud_index == i;

                if (ImGui::Selectable(LP::baud_rates[i].str, selected)) {
                    combobox_baud_index = i;
                    ImGui::SetItemDefaultFocus();
                }

            }
            ImGui::EndCombo();
        }

        ImGui::EndTable();
    }

    // ====== Buttons ======
    if (ImGui::BeginTable("Buttons", 3)) {
        ImGui::TableSetupColumn("Left", ImGuiTableColumnFlags_WidthStretch, 0.3f);
        ImGui::TableSetupColumn("Center", ImGuiTableColumnFlags_WidthStretch, 0.3f);
        ImGui::TableSetupColumn("Right", ImGuiTableColumnFlags_WidthStretch, 0.3f);

        ImGui::TableNextRow();
        ImGui::TableNextColumn();

        // calc buttons size        
        ImVec2 buttons_size;
        buttons_size.x = buttons_size.y = ICON_SIZE + (ICON_SIZE / 6.0f) * 2;

        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (ImGui::GetContentRegionAvail().x - buttons_size.x) * 0.5);
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, ICON_SIZE);
        refresh_button = ImGui::Button(ICON_LC_REFRESH, buttons_size);

        ImGui::TableNextColumn();
        
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (ImGui::GetContentRegionAvail().x - buttons_size.x) * 0.5);
        open_close_button = ImGui::Button(app_state == READING ? ICON_LC_PAUSE : ICON_LC_PLAY, buttons_size);
        
        ImGui::TableNextColumn();
        
        if (no_telemetry) {
            ImGui::BeginDisabled();
        }
    
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (ImGui::GetContentRegionAvail().x - buttons_size.x) * 0.5);
        save_button = ImGui::Button(ICON_LC_SAVE, buttons_size);
        ImGui::PopStyleVar();
        
        if (no_telemetry) {
            ImGui::EndDisabled();
        }

        ImGui::EndTable();
    }

    // stop reading if the Save button has been pressed
    if (app_state == READING && save_button) {
        open_close_button = true;
    }

    // =========== ADVANCED SERIAL CONFIG =========== 
    if (ImGui::TreeNode("Advanced configuration")) {

        if (app_state == READING) {
            ImGui::BeginDisabled();
        }

        uint16_t parity      = Serial::get_parity();
        uint16_t stop_bits   = Serial::get_stop_bits();
        uint16_t data_bits   = Serial::get_data_bits();
        uint16_t flow_ctrl   = Serial::get_flow_ctrl();

        // === Parity bit ===
        ImGui::Text("Parity:");

        if (ImGui::RadioButton("Disabled###ParityDIS", !parity)) {
            parity = 0;
        }

        if (ImGui::RadioButton("Even###ParityEV", parity == 1)) {
            parity = 1;
        }

        if (ImGui::RadioButton("Odd###ParityODD", parity == 2)) {
            parity = 2;
        }

        // === Stop bits ===
        ImGui::Text("Stop bits:");

        if (ImGui::RadioButton("1###Stop1", stop_bits == 1)) {
            stop_bits = 1;
        }

        ImGui::SameLine();

        if (ImGui::RadioButton("2###Stop2", stop_bits == 2)) {
            stop_bits = 2;
        }

        // === Data bits ===
        ImGui::Text("Data bits:");

        if (ImGui::RadioButton("5###Data1", data_bits == LP_CS5)) {
            data_bits = LP_CS5;
        }

        ImGui::SameLine();

        if (ImGui::RadioButton("6###Data2", data_bits == LP_CS6)) {
            data_bits = LP_CS6;
        }

        if (ImGui::RadioButton("7###Data3", data_bits == LP_CS7)) {
            data_bits = LP_CS7;
        }

        ImGui::SameLine();

        if (ImGui::RadioButton("8###Data4", data_bits == LP_CS8)) {
            data_bits = LP_CS8;
        }

        // === Flow ctrl ===
        ImGui::Text("Flow control:");

        if (ImGui::RadioButton("Disabled###FlowDIS", !flow_ctrl)) {
            flow_ctrl = 0;
        }

        if (ImGui::RadioButton("RTS/CTS###FlowHW", flow_ctrl == 1)) {
            flow_ctrl = 1;
        }

        if (ImGui::RadioButton("XON/XOFF###FlowSW", flow_ctrl == 2)) {
            flow_ctrl = 2;
        }

        Serial::set_parity(parity);
        Serial::set_stop_bits(stop_bits);
        Serial::set_data_bits(data_bits);
        Serial::set_flow_ctrl(flow_ctrl);

        if (app_state == READING) {
            ImGui::EndDisabled();
        }

        ImGui::TreePop();
    }
}
