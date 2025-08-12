#include <algorithm>

#include <libreborn/util/string.h>

#include "../configuration.h"

// Utility Function
static std::string get_label_for_flag_node(const FlagNode &node) {
    return node.name + "##FlagNode" + safe_to_string(node.id);
}

// Advanced Tab
void ConfigurationUI::draw_advanced() {
    // Search
    ImGui::SetNextItemWidth(-FLT_MIN);
    if (ImGui::InputTextWithHint("##Filter", "Search", filter.InputBuf, IM_ARRAYSIZE(filter.InputBuf))) {
        filter.Build();
    }

    // Features
    if (ImGui::BeginChild("Features", ImVec2(0, 0), ImGuiChildFlags_Borders, ImGuiWindowFlags_HorizontalScrollbar)) {
        // Top-Level Categories
        for (FlagNode &category : state.flags.root.children) {
            if (!should_draw_category(category)) {
                continue;
            }
            const std::string label = get_label_for_flag_node(category);
            if (ImGui::CollapsingHeader(label.c_str())) {
                draw_category(category);
            }
        }
    }
    ImGui::EndChild();
}

// Draw Flag Category
void ConfigurationUI::draw_category(FlagNode &category) {
    for (FlagNode &child : category.children) {
        if (!should_draw_category(child)) {
            continue;
        }
        const std::string label = get_label_for_flag_node(child);
        if (!child.children.empty()) {
            // Sub-Category
            if (ImGui::TreeNodeEx(label.c_str(), ImGuiTreeNodeFlags_SpanAvailWidth)) {
                draw_category(child);
                ImGui::TreePop();
            }
        } else {
            // Flag
            ImGui::Checkbox(label.c_str(), &child.value);
        }
    }
}
bool ConfigurationUI::should_draw_category(const FlagNode &category) {
    if (category.children.empty()) {
        // Flag
        return filter.PassFilter(category.name.c_str());
    } else {
        // Recursive
        return std::ranges::any_of(category.children, [this](const FlagNode &child) {
            return should_draw_category(child);
        });
    }
}