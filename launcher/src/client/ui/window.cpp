#include "../configuration.h"

// Construct
static constexpr int size = 400;
ConfigurationUI::ConfigurationUI(State &state_, bool &save_settings_):
    Frame("Launcher", size, size),
    original_state(state_),
    state(state_),
    save_settings(save_settings_) {}

// Render
int ConfigurationUI::render() {
    if (ImGui::BeginChild("Main", ImVec2(0, -ImGui::GetFrameHeightWithSpacing() /* Leave Room For Bottom Row */), ImGuiChildFlags_None, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse)) {
        // Tabs
        if (ImGui::BeginTabBar("TabBar")) {
            // Main Tab
            if (ImGui::BeginTabItem("General")) {
                draw_main();
                ImGui::EndTabItem();
            }
            // Advanced Tab
            if (ImGui::BeginTabItem("Advanced")) {
                draw_advanced();
                ImGui::EndTabItem();
            }
            // Servers Tab
            if (ImGui::BeginTabItem("Servers")) {
                draw_servers();
                ImGui::EndTabItem();
            }
            // About Tab
            if (ImGui::BeginTabItem("About")) {
                draw_about();
                ImGui::EndTabItem();
            }
            ImGui::EndTabBar();
        }
    }
    ImGui::EndChild();
    // Bottom Row
    return draw_bottom();
}

// Bottom Row
int ConfigurationUI::draw_bottom() const {
    // Reset Settings
    const State default_state;
    std::vector<std::tuple<std::string, std::string, const State *>> reset_options = {
        {"Revert", "Last Saved", &original_state},
        {"Reset", "Default", &default_state},
    };
    for (const std::tuple<std::string, std::string, const State *> &option : reset_options) {
        const State &new_state = *std::get<2>(option);
        ImGui::BeginDisabled(state == new_state);
        if (ImGui::Button(std::get<0>(option).c_str())) {
            state = new_state;
        }
        ImGui::SetItemTooltip("Use %s Settings", std::get<1>(option).c_str());
        ImGui::EndDisabled();
        ImGui::SameLine();
    }

    // Right-Align Buttons
    int ret = 0;
    draw_right_aligned_buttons({quit_text, "Launch"}, [&ret](const int id, const bool was_clicked) {
        if (id == 0) {
            // Quit
            if (was_clicked) {
                ret = -1;
            }
            ImGui::SetItemTooltip("Changes Will Not Be Saved!");
        } else if (was_clicked) {
            // Launch
            ret = 1;
        }
    });

    // Return
    return ret;
}