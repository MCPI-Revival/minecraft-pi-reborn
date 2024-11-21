#include <vector>

#include "configuration.h"
#include "cache.h"

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
ConfigurationUI::ConfigurationUI(State &state_):
    Frame("Launcher", size, size),
    state(state_) {
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
    const ImGuiStyle &style = ImGui::GetStyle();
    if (ImGui::BeginChild("General", ImVec2(0, -ImGui::GetFrameHeightWithSpacing() /* Leave Room For One Line */), ImGuiChildFlags_None, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse)) {
        // Tabs
        if (ImGui::BeginTabBar("tab_bar", ImGuiTabBarFlags_None)) {
            if (ImGui::BeginTabItem("General", nullptr, ImGuiTabItemFlags_None)) {
                // Main Tab
                draw_main();
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Advanced", nullptr, ImGuiTabItemFlags_None)) {
                // Advanced Tab
                draw_advanced();
                ImGui::EndTabItem();
            }
            ImGui::EndTabBar();
        }
    }
    ImGui::EndChild();
    // Bottom Row
    if (ImGui::Button("Reset To Defaults")) {
        state = State(empty_cache);
    }
    ImGui::SameLine();
    // Right-Align Buttons
    const char *bottom_row_text[] = {"Quit", "Launch"};
    float width_needed = 0;
    for (const char *text : bottom_row_text) {
        if (width_needed > 0) {
            width_needed += style.ItemSpacing.x;
        }
        width_needed += ImGui::CalcTextSize(text).x + style.FramePadding.x * 2.f;
    }
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x - width_needed);
    if (ImGui::Button(bottom_row_text[0])) {
        // Quit
        return -1;
    }
    ImGui::SameLine();
    if (ImGui::Button(bottom_row_text[1])) {
        // Launch
        return 1;
    }
    // Return
    return 0;
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
}

// Advanced Tab
static std::string get_label(const FlagNode &node) {
    return node.name + "##" + std::to_string(node.id);
}
void ConfigurationUI::draw_advanced() const {
    if (ImGui::BeginChild("Features", ImVec2(0, 0), ImGuiChildFlags_Borders, ImGuiWindowFlags_HorizontalScrollbar)) {
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