#include "Toolbar.hpp"
#include "LambdaApp.hpp"
#include "imgui.h"

namespace LP {
    void Toolbar::on_update(double dt) {

    }

    void Toolbar::on_render() {
        ImGuiWindowFlags win_flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoNav;

        if (ImGui::Begin("Toolbar", nullptr, win_flags)) {
            if (ImGui::BeginTabBar("##ToolbarTab")) {

                if (ImGui::BeginTabItem("Serial"))
                {
                    active_protocol_tab = SERIAL;
                    ImGui::Text("This is the Serial tab!\nblah blah blah blah blah");
                    ImGui::EndTabItem();
                }
                if (ImGui::BeginTabItem("UDP"))
                {
                    active_protocol_tab = UDP;
                    ImGui::Text("This is the UDP tab!\nblah blah blah blah blah");
                    ImGui::EndTabItem();
                }

                auto app_state = LambdaApp::get().get_app_state();

                if (app_state == IDLE)
                    ImGui::Text("IDLE");
                else
                    ImGui::Text("RUNNING");
                
                ImGui::EndTabBar();
            }
        }
        ImGui::End();
    }

    void Toolbar::render_serial() {
        
    }

    void Toolbar::render_udp() {

    }
}
