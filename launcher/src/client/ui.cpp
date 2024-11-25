#include <vector>

#include "configuration.h"

#include <imgui_stdlib.h>

// Render Distances
static std::vector render_distances = {
    "Far",
    "Normal",
    "Short",
    "Tiny"
};

// Construct
static constexpr int size = 400;
ConfigurationUI::ConfigurationUI(State &state_, bool &save_settings_):
    Frame("Launcher", size, size),
    original_state(state_),
    state(state_),
    save_settings(save_settings_) {
    update_render_distance();
}
void ConfigurationUI::update_render_distance() {
    render_distance_index = 0;
    for (std::vector<std::string>::size_type i = 0; i < render_distances.size(); i++) {
        if (std::string(render_distances[i]) == state.render_distance) {
            render_distance_index = int(i);
            break;
        }
    }
}

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
            ImGui::EndTabBar();
        }
    }
    ImGui::EndChild();
    // Bottom Row
    return draw_bottom();
}

// Bottom Row
int ConfigurationUI::draw_bottom() {
    // Reset All Settings
    const State default_state;
    std::vector<std::tuple<const char *, const char *, const State *>> reset_options = {
        {"Revert", "Last Saved", &original_state},
        {"Reset", "Default", &default_state},
    };
    for (const std::tuple<const char *, const char *, const State *> &option : reset_options) {
        const State &new_state = *std::get<2>(option);
        ImGui::BeginDisabled(state == new_state);
        if (ImGui::Button(std::get<0>(option))) {
            state = new_state;
            update_render_distance();
        }
        ImGui::SetItemTooltip("Use %s Settings", std::get<1>(option));
        ImGui::EndDisabled();
        ImGui::SameLine();
    }
    // Right-Align Buttons
    int ret = 0;
    draw_right_aligned_buttons({"Quit", "Launch"}, [&ret](const int id, const bool was_clicked) {
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

// Main Tab
void ConfigurationUI::draw_main() {
    const ImGuiStyle &style = ImGui::GetStyle();
    const char *labels[] = {"Username", "Render Distance"};
    // Calculate Label Size
    float label_size = 0;
    for (const char *label : labels) {
        label_size = std::max(label_size, ImGui::CalcTextSize(label).x + style.ItemInnerSpacing.x);
    }
    ImGui::PushItemWidth(-label_size);
    // Options
    ImGui::InputText(labels[0], &state.username);
    ImGui::Combo(labels[1], &render_distance_index, render_distances.data(), int(render_distances.size()));
    state.render_distance = render_distances[render_distance_index];
    ImGui::PopItemWidth();
    ImGui::Checkbox("Save Settings On Launch", &save_settings);
}

// Advanced Tab
static std::string get_label(const FlagNode &node) {
    return node.name + "##" + std::to_string(node.id);
}
void ConfigurationUI::draw_advanced() const {
    if (ImGui::BeginChild("Features", ImVec2(0, 0), ImGuiChildFlags_Borders, ImGuiWindowFlags_HorizontalScrollbar)) {
        // Categories
        for (FlagNode &category : state.flags.root.children) {
            std::string label = get_label(category);
            if (ImGui::CollapsingHeader(label.c_str())) {
                draw_category(category);
            }
        }
    }
    ImGui::EndChild();
}

// Feature Categories
void ConfigurationUI::draw_category(FlagNode &category) {
    for (FlagNode &child : category.children) {
        std::string label = get_label(child);
        if (!child.children.empty()) {
            if (ImGui::TreeNode(label.c_str())) {
                draw_category(child);
                ImGui::TreePop();
            }
        } else {
            ImGui::Checkbox(label.c_str(), &child.value);
        }
    }
}