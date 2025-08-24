#ifndef __PLOT_VIEW_H__
#define __PLOT_VIEW_H__

#include "telemetry.h"
#include <imgui.h>
#include <unordered_map>

namespace BSP {
    typedef void (*PlotFunc)(const char*, const double*, const double*, int, int, int, int);
    typedef int key_t;

    typedef struct ChannelStyle {
        size_t combobox_func_index = 0;
        ImVec4 color;
        bool   show;
    } ChannelStyle;

    typedef struct PlotStyle {
        PlotTimeStyle time_style = DATETIME;
        Limits    limits;
    } PlotStyle;

    typedef struct plot_functions_t {
        const char* str;
        PlotFunc    func;
    } plot_functions_t;

    extern const plot_functions_t plot_functions[];
    extern const size_t           plot_functions_size;

    // `PlotView` is the class responsible of the UI components regarding data plotting and it's settings.
    class PlotView {
        private:
            std::unordered_map<key_t, ChannelStyle> plot_attributes;
            PlotStyle plot_style;
            size_t    combobox_time_index;
            bool      clear_button;

            // initialize a new plot line style with key `id`, or overwrite if the key exists
            void channelStyleInit(key_t id);
            
            void renderChannelSettings(key_t id, PlotData& data, ChannelStyle& style);
        public:
            PlotView() : combobox_time_index(2), clear_button(false) {}

            void renderPlot(Telemetry& tel, app_state_t app_state, int pos_x, int pos_y, int width, int height);

            void renderTelemetryToolbar(Telemetry& tel);

            void renderPlotOptions();

            void renderDataFormat(Telemetry& tel, app_state_t app_state);

            PlotStyle getPlotStyle() { return plot_style; }
    };
}

#endif
