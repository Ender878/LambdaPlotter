#ifndef __PLOT_VIEW_H__
#define __PLOT_VIEW_H__

#include "telemetry.h"
#include <imgui.h>
#include <unordered_map>

#define PLOT_FUNC_SIZE 3

namespace LP {
    typedef void (*PlotFunc)(const char*, const double*, const double*, int, int, int, int);

    // struct storing channels' plot style
    typedef struct ChannelStyle {
        size_t combobox_func_index = 0;
        ImVec4 color;
        bool   show;
    } ChannelStyle;

    // general plot attributes
    typedef struct PlotStyle {
        PlotTimeStyle time_style = DATETIME;
        Limits        limits;
    } PlotStyle;

    typedef struct plot_functions_t {
        const char* str;
        PlotFunc    func;
    } plot_functions_t;

    extern const std::array<plot_functions_t, PLOT_FUNC_SIZE> plot_functions;

    // `PlotView` is the class responsible for the UI components regarding data plotting and it's settings.
    class PlotView {
        private:
            std::unordered_map<int, ChannelStyle> plot_attributes;
            PlotStyle plot_style;
            size_t    combobox_time_index;

            /**
             * @brief initialize a new channel plot style with key `id`, or overwrite it if the key already exists
             * 
             * @param id channel id
             */
            void channel_style_init(int id);
            
            /**
             * @brief Renders a pop-up containing various channel configuration widgets
             * 
             * @param id    channel id
             * @param data  channel
             * @param style channel style
             */
            void render_channel_settings(int id, Channel& data, ChannelStyle& style);

            /**
             * @brief Render a tooltip hoverable widget containing text
             * 
             * @param message the string to be displayed
             */
            void render_tooltip(const char* message);
        public:
            PlotView() : plot_style(), combobox_time_index(2) {}

          /**
           * @brief Render the plot
           *
           * @param tel         Reference to a telemetry object containing all the channel's data
           * @param app_state   The current app state (READING or IDLE)
           * @param pos_x       X position for the plot window
           * @param pos_y       Y position fot the plot window
           * @param width
           * @param height
           */
            void render_plot(Telemetry& tel, app_state_t app_state, float pos_x, float pos_y, float width, float height);

            /**
             * @brief Render widgets used to interact and read received data 
             * 
             * @param tel Telemetry object containing all the channels' data
             */
            void render_telemetry(Telemetry& tel);

            /**
             * @brief Render widgets used for changing general plot options
             * 
             */
            void render_plot_options();

            /**
             * @brief Render the form used to create the data frame format
             * 
             * @param tel       Telemetry object containing all the channels' data
             * @param app_state The current app state (READING or IDLE)
             */
            void render_data_format(Telemetry& tel, app_state_t app_state);

            /**
             * @brief Get the plot style object
             * 
             * @return PlotStyle 
             */
            PlotStyle get_plot_style() { return plot_style; }

            /**
             * @brief Get the channels style object
             * 
             * @return std::unordered_map<int, ChannelStyle> 
             */
            std::unordered_map<int, ChannelStyle> get_channels_style() { return plot_attributes; }
    };
}

#endif
