#include <BSP/plotView.h>
#include <BSP/telemetry.h>
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <format>
#include <imgui.h>
#include <imgui_internal.h>
#include <mutex>
#include <string>
#include "../fonts/lucide.h"
#include "../implot/implot.h"
#include "common/shared.h"

#define TEXT_BUFFER_SIZE 256

void BSP::PlotView::renderPlot(Telemetry& tel, app_state_t app_state, int pos_x, int pos_y, int width, int height) {
    std::lock_guard<std::mutex> lock(BSP::plot_mtx);

    std::vector<double>* times = (plot_style.time_style == DATETIME) ? tel.get_unix_timestamps() : tel.get_elapsed_timestamps();
    auto* data = tel.getData();

    ImGui::SetNextWindowPos(ImVec2(pos_x, pos_y));
    ImGui::SetNextWindowSize(ImVec2(width, height));
    if (ImGui::Begin("##plot", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse)) {
        if (!data->empty()) {
            if (ImPlot::BeginPlot("##plot_win", ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y))) {
                    ImPlot::SetupAxis(ImAxis_X1, "Time");

                    ImPlot::SetupAxis(ImAxis_Y1, "##Data", (app_state == READING) ? ImPlotAxisFlags_AutoFit | ImPlotAxisFlags_RangeFit : 0);

                    if (plot_style.time_style == DATETIME) {
                        ImPlot::SetupAxisScale(ImAxis_X1, ImPlotScale_Time);
                    }

                    ImPlot::SetupLegend(ImPlotLocation_NorthWest, ImPlotLegendFlags_NoButtons);
    
                    double first_time = times->front();
                    double last_time  = times->back();
    
                    int time_window = BSP::time_windows[combobox_time_index].value;

                    if (plot_style.time_style == ELAPSED) {
                        time_window *= 1000;
                    }
                    
                    // Set time window, if there is.
                    double window_start = (time_window != 0)
                        ? std::max(first_time, last_time - time_window)
                        : first_time;
    
                    if (app_state == READING) // TODO: fix time format change
                        ImPlot::SetupAxisLimits(ImAxis_X1, window_start, last_time, ImGuiCond_Always);

                    // get plot window limits and set the time window (used when saving the plot)
                    ImPlotRect limits = ImPlot::GetPlotLimits(ImAxis_X1, ImAxis_Y1);
                    plot_style.limits = {
                        limits.Min().x,
                        limits.Max().x,
                        limits.Min().y,
                        limits.Max().y
                    };

                    for (auto& stream : *data) {
                        auto  data_id = stream.first;
                        auto& channel = stream.second;

                        if (!plot_attributes.contains(data_id)) {
                            channelStyleInit(data_id);
                        }

                        if (plot_attributes[data_id].show) {
                            std::string label = std::format("{}##{}", channel.name, data_id);

                            ImPlot::PushStyleColor(ImPlotCol_Line, plot_attributes[data_id].color);
                            ImPlot::PushStyleColor(ImPlotCol_Fill, plot_attributes[data_id].color);

                            if (channel.offset != channel.prev_offset || channel.scale != channel.prev_scale) {
                                channel.values_transformed.clear();
                                for (size_t i = 0; i < channel.values.size(); i++) {
                                    channel.values_transformed.push_back((channel.values[i] * channel.scale) + channel.offset);
                                }
                            }

                            channel.prev_offset = channel.offset;
                            channel.prev_scale  = channel.scale;

                            plot_functions[plot_attributes[data_id].combobox_func_index].func(label.c_str(), times->data(), channel.values_transformed.data(), channel.values.size(), 0, 0, sizeof(double));

                            ImPlot::PopStyleColor(2);
                        }
                    }

                ImPlot::EndPlot();
            }
        } else {
            ImPlot::BeginPlot("##plot_win", ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y));
            ImPlot::EndPlot();
        }

        ImGui::End();
    }
}

void BSP::PlotView::renderTelemetryToolbar(Telemetry& tel) {
    ImGui::SeparatorText("Telemetry");

    static bool open = false;

    if (!tel.is_empty()) {
        if (ImGui::BeginTable("##TelTable", 4)) {
            ImGui::TableSetupColumn("Color",  ImGuiTableColumnFlags_WidthStretch, 0.4f);
            ImGui::TableSetupColumn("Value",  ImGuiTableColumnFlags_WidthStretch, 0.3f);
            ImGui::TableSetupColumn("Settings", ImGuiTableColumnFlags_WidthStretch, 0.1f);
            ImGui::TableSetupColumn("Button", ImGuiTableColumnFlags_WidthFixed, 50.0f);
    
            std::lock_guard<std::mutex> lock(plot_mtx);
            for (auto& data : *tel.getData()) {
                auto data_id = data.first;
        
                if (!plot_attributes.contains(data_id)) {
                    channelStyleInit(data_id);
                }
    
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
    
                ImGui::PushID(data_id);
        
                float selected_color[4] = { plot_attributes[data_id].color.x, plot_attributes[data_id].color.y, plot_attributes[data_id].color.z, plot_attributes[data_id].color.w };
                ImGui::ColorEdit4("##Color", selected_color, ImGuiColorEditFlags_NoInputs);
                plot_attributes[data_id].color = { selected_color[0], selected_color[1], selected_color[2], selected_color[3] };
        
                ImGui::SameLine();
    
                // create the text buffer
                char display_name_buffer[TEXT_BUFFER_SIZE];
                std::strncpy(display_name_buffer, data.second.name.c_str(), sizeof(display_name_buffer) - 1);
                display_name_buffer[sizeof(display_name_buffer) - 1] = '\0';
    
                ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
                ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);
    
                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                ImGui::InputText("##Name", display_name_buffer, sizeof(display_name_buffer));
                data.second.name = display_name_buffer;
    
                // if the name is an empty string, assign the default name
                if (data.second.name.empty()) {
                    data.second.name = std::format("Data {}", data_id);
                }
    
                ImGui::PopStyleVar();
                ImGui::PopStyleColor();
    
                ImGui::TableNextColumn();
    
                if (!data.second.values_transformed.empty()) {
                    ImGui::Text("%.2f", data.second.values_transformed.back());
                }
                
                ImGui::TableNextColumn();
        
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
                ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);
                ImGui::Button(ICON_LC_SETTINGS);
                ImGui::PopStyleColor(3);
                ImGui::PopStyleVar();

                if (ImGui::IsItemClicked()) {
                    ImGui::OpenPopup(("##ChannelConfig" + std::to_string(data_id)).c_str());
                }

                renderChannelSettings(data_id, data.second, plot_attributes[data_id]);
        
                ImGui::TableNextColumn();
        
                std::string bt_id = std::format("{}", plot_attributes[data_id].show ? "HIDE" : "SHOW");
                if (ImGui::Button(bt_id.c_str())) {
                    plot_attributes[data_id].show = !plot_attributes[data_id].show;
                    open = !open;
                }

                ImGui::PopID();
            }
    
            ImGui::EndTable();
        }

        if (ImGui::Button("Clear")) {
            std::lock_guard<std::mutex> lock(BSP::plot_mtx);
            tel.clearValues();
        }
    } else {
        ImGui::Text("No telemetry received yet!");
    }    
}

void BSP::PlotView::renderPlotOptions() {
    ImGui::SeparatorText("Plot options");

    if (ImGui::BeginTable("##TimeTable", 2)) {
        ImGui::TableSetupColumn("Label",  ImGuiTableColumnFlags_WidthStretch, 0.4f);
        ImGui::TableSetupColumn("Combo",  ImGuiTableColumnFlags_WidthStretch, 0.6f);

        ImGui::TableNextRow();
        ImGui::TableNextColumn();

        ImGui::Text("Time Window");

        ImGui::TableNextColumn();

        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
        if (ImGui::BeginCombo("##time", BSP::time_windows[combobox_time_index].str)) {
            for (size_t i = 0; i < BSP::time_windows_size; i++) {
                bool selected = combobox_time_index == i;

                if (ImGui::Selectable(BSP::time_windows[i].str, selected)) {
                    combobox_time_index = i;
                    ImGui::SetItemDefaultFocus();
                }
            }
            ImGui::EndCombo();
        }

        ImGui::TableNextRow();
        ImGui::TableNextColumn();

        ImGui::Text("Time Format:");

        ImGui::TableNextRow();
        ImGui::TableNextColumn();

        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize("Datetime").x) * 0.5f);
        if (ImGui::RadioButton("Datetime", plot_style.time_style == DATETIME)) {
            plot_style.time_style = DATETIME;
        }

        ImGui::TableNextColumn();

        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize("Time Elapsed").x) * 0.5f);
        if (ImGui::RadioButton("Time elapsed", plot_style.time_style == ELAPSED)) {
            plot_style.time_style = ELAPSED;
        }

        ImGui::EndTable();
    }
}

void BSP::PlotView::channelStyleInit(key_t id) {
    ImVec4 default_colormap = ImPlot::GetColormapColor(id - 1);
    plot_attributes[id] = {
        .color = { default_colormap.x, default_colormap.y, default_colormap.z, default_colormap.w },
        .show  = true
    };
}

void BSP::PlotView::renderChannelSettings(key_t id, Channel& data, ChannelStyle& style) {
    if (ImGui::BeginPopup(("##ChannelConfig" + std::to_string(id)).c_str())) {
        ImGui::Text("Channel '%s'", data.name.c_str());
        ImGui::Separator();

        if (ImGui::BeginTable("##ChannelConfigTable", 2)) {
            ImGui::TableSetupColumn("Label",  ImGuiTableColumnFlags_WidthStretch, 0.4f);
            ImGui::TableSetupColumn("Item",  ImGuiTableColumnFlags_WidthStretch, 0.6f);

            ImGui::TableNextRow();
            ImGui::TableNextColumn();

            ImGui::Text("Scale: ");
    
            ImGui::TableNextColumn();
    
            ImGui::SetNextItemWidth(120.0f);
            ImGui::InputDouble("##Scale", &data.scale, 0.1, 0.5, "%.2f");

            ImGui::TableNextRow();
            ImGui::TableNextColumn();

            ImGui::Text("Offset: ");

            ImGui::TableNextColumn();

            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
            ImGui::InputDouble("##offset", &data.offset, 0.1, 0.5, "%.02f");
            
            ImGui::TableNextRow();
            ImGui::TableNextColumn();

            // line style (line, scatter, bars, shaded), thickness 
            ImGui::Text("Line Style: ");
    
            ImGui::TableNextColumn();
    
            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
            if (ImGui::BeginCombo("##func", BSP::plot_functions[style.combobox_func_index].str)) {
                for (size_t i = 0; i < BSP::plot_functions_size; i++) {
                    bool selected = style.combobox_func_index == i;
    
                    if (ImGui::Selectable(BSP::plot_functions[i].str, selected)) {
                        style.combobox_func_index = i;
                        ImGui::SetItemDefaultFocus();
                    }
    
                }
                ImGui::EndCombo();
            }

            ImGui::EndTable();
        }

        ImGui::EndPopup();
    }
}

void BSP::PlotView::renderDataFormat(Telemetry& tel, app_state_t app_state) {
    ImGui::SeparatorText("Data Format");

    if (app_state == READING) {
        ImGui::BeginDisabled();
    }

    // check if there are separators with the same value. This is for letting the user know that the serial data will
    // not be parsed correctly.
    bool same_str_channel_sep = std::strcmp(tel.frame_format.channel_sep, tel.frame_format.frame_end) == 0 || 
                                ((tel.frame_format.named) ? 
                                std::strcmp(tel.frame_format.channel_sep, tel.frame_format.name_sep)  == 0 : 0);

    bool same_str_frame_end   = std::strcmp(tel.frame_format.frame_end, tel.frame_format.channel_sep) == 0 || 
                                ((tel.frame_format.named) ? 
                                std::strcmp(tel.frame_format.frame_end, tel.frame_format.name_sep)    == 0 : 0);

    bool same_str_name_sep    = (tel.frame_format.named) ? std::strcmp(tel.frame_format.name_sep, tel.frame_format.channel_sep)  == 0 || 
                                                           std::strcmp(tel.frame_format.name_sep, tel.frame_format.frame_end)    == 0 : 0;

    if (ImGui::BeginTable("##format_table", 2)) {    
        ImGui::TableSetupColumn("Label", ImGuiTableColumnFlags_WidthStretch, 0.6f);
        ImGui::TableSetupColumn("Item",  ImGuiTableColumnFlags_WidthStretch, 0.4f);

        ImGui::TableNextRow();
        ImGui::TableNextColumn();

        ImGui::Text("Channel Separator:");

        ImGui::TableNextColumn();

        if (same_str_channel_sep) {
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(1.0, 0.0f, 0.0f, 1.0f));
        }

        ImGui::InputText("##channel_sep", tel.frame_format.channel_sep, sizeof(tel.frame_format.channel_sep));
        tel.frame_format.channel_sep[sizeof(tel.frame_format.channel_sep) - 1] = '\0';

        if (same_str_channel_sep) {
            ImGui::PopStyleColor();
        }

        ImGui::SameLine();

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.7f, 0.7f, 0.7f, 1.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);
        ImGui::SmallButton("(?)");
        ImGui::PopStyleColor(4);
        ImGui::PopStyleVar();

        if (ImGui::IsItemHovered()) {
            ImGui::BeginTooltip();
            // TODO: finish this
            ImGui::Text("Ciaoo");
        
            ImGui::EndTooltip();
        }

        ImGui::TableNextRow();
        ImGui::TableNextColumn();

        ImGui::Text("Frame end:");               
    
        ImGui::TableNextColumn();

        if (same_str_frame_end) {
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(1.0, 0.0f, 0.0f, 1.0f));
        }

        ImGui::InputText("##frame_end", tel.frame_format.frame_end, sizeof(tel.frame_format.frame_end));
        tel.frame_format.frame_end[sizeof(tel.frame_format.frame_end) - 1] = '\0';

        if (same_str_frame_end) {
            ImGui::PopStyleColor();
        }

        ImGui::TableNextRow();
        ImGui::TableNextColumn();

        ImGui::Text("Named:");

        ImGui::TableNextColumn();

        ImGui::Checkbox("##named", &tel.frame_format.named);

        if (tel.frame_format.named) {
            ImGui::TableNextRow();
            ImGui::TableNextColumn();

            ImGui::Text("Name separator:");

            ImGui::TableNextColumn();

            if (same_str_name_sep) {
                ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(1.0, 0.0f, 0.0f, 1.0f));
            }

            ImGui::InputText("##name_sep", tel.frame_format.name_sep, sizeof(tel.frame_format.name_sep));
            tel.frame_format.name_sep[sizeof(tel.frame_format.name_sep) - 1] = '\0';

            if (same_str_name_sep) {
                ImGui::PopStyleColor();
            }
        }

        ImGui::EndTable();
    }

    ImGui::Text("Preview:");

    // check if the values are empty
    if (std::strcmp(tel.frame_format.channel_sep, "") == 0) {
        std::strcpy(tel.frame_format.channel_sep, " ");
    }

    if (std::strcmp(tel.frame_format.frame_end, "") == 0) {
        std::strcpy(tel.frame_format.frame_end, "\\n");
    }

    if (std::strcmp(tel.frame_format.name_sep, "") == 0) {
        std::strcpy(tel.frame_format.name_sep, ":");
    }

    float prev_size = (2 * ImGui::CalcTextSize(tel.frame_format.channel_sep).x + 3 * ImGui::CalcTextSize("X").x + ImGui::CalcTextSize(tel.frame_format.frame_end).x);
    
    if (tel.frame_format.named) {
        prev_size += 3 * ImGui::CalcTextSize("N").x + 3 * ImGui::CalcTextSize(tel.frame_format.name_sep).x;
    }

    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (ImGui::GetContentRegionAvail().x - prev_size) * 0.5);

    if (tel.frame_format.named) {
        ImGui::TextColored(ImVec4(108 / 255.0f, 141 / 255.0f, 176 / 255.0f, 1.0f), "N");

        ImGui::SameLine(0, 0);

        ImGui::Text("%s", tel.frame_format.name_sep);

        ImGui::SameLine(0, 0);
    }

    ImGui::TextColored(ImVec4(44 / 255.0f, 180 / 255.0f, 44 / 255.0f, 1.0f), "X");

    ImGui::SameLine(0, 0);

    ImGui::Text("%s", tel.frame_format.channel_sep);

    ImGui::SameLine(0, 0);

    if (tel.frame_format.named) {
        ImGui::TextColored(ImVec4(108 / 255.0f, 141 / 255.0f, 176 / 255.0f, 1.0f), "N");

        ImGui::SameLine(0, 0);

        ImGui::Text("%s", tel.frame_format.name_sep);

        ImGui::SameLine(0, 0);
    }

    ImGui::TextColored(ImVec4(44 / 255.0f, 180 / 255.0f, 44 / 255.0f, 1.0f), "Y");

    ImGui::SameLine(0, 0);

    ImGui::Text("%s", tel.frame_format.channel_sep);

    ImGui::SameLine(0, 0);

    if (tel.frame_format.named) {
        ImGui::TextColored(ImVec4(108 / 255.0f, 141 / 255.0f, 176 / 255.0f, 1.0f), "N");

        ImGui::SameLine(0, 0);

        ImGui::Text("%s", tel.frame_format.name_sep);

        ImGui::SameLine(0, 0);
    }

    ImGui::TextColored(ImVec4(44 / 255.0f, 180 / 255.0f, 44 / 255.0f, 1.0f), "Z");

    ImGui::SameLine(0, 0);

    ImGui::TextColored(ImVec4(1.0f, 165 / 255.0f, 0.0f, 1.0f), "%s", tel.frame_format.frame_end);

    if (same_str_channel_sep || same_str_frame_end || same_str_name_sep) {
        ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Duplicate tokens, Data won't be parsed correctly!");
    }

    if (app_state == READING) {
        ImGui::EndDisabled();
    }
}

const BSP::plot_functions_t BSP::plot_functions[] = {
    {"Line",    ImPlot::PlotLine<double>    },
    {"Scatter", ImPlot::PlotScatter<double> },
    {"Shaded",  ImPlot::PlotShaded<double>  },
};

constexpr size_t BSP::plot_functions_size = sizeof(BSP::plot_functions) / sizeof(*BSP::plot_functions);
