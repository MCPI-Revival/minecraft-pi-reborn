#include <algorithm>

#include <libreborn/util/string.h>
#include <libreborn/log.h>

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
            if (!category.is_category()) {
                IMPOSSIBLE();
            }
            const std::string label = get_label_for_flag_node(category);
            const bool ret = ImGui::CollapsingHeader(label.c_str());
            add_flag_tooltip(category);
            if (ret) {
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
        if (child.is_category()) {
            // Sub-Category
            const bool ret = ImGui::TreeNodeEx(label.c_str(), ImGuiTreeNodeFlags_SpanAvailWidth);
            add_flag_tooltip(child);
            if (ret) {
                draw_category(child);
                ImGui::TreePop();
            }
        } else {
            // Flag
            ImGui::BeginDisabled(child.is_advanced && !ImGui::GetIO().KeyShift);
            ImGui::Checkbox(label.c_str(), &child.value.value());
            add_flag_tooltip(child);
            ImGui::EndDisabled();
        }
    }
}
bool ConfigurationUI::should_draw_category(const FlagNode &category) {
    if (!category.is_category()) {
        // Flag
        return filter.PassFilter(category.name.c_str());
    } else {
        // Recursive
        return std::ranges::any_of(category.children, [this](const FlagNode &child) {
            return should_draw_category(child);
        });
    }
}

// Tooltips
void ConfigurationUI::add_flag_tooltip(const FlagNode &flag) {
    // Handle 'Advanced" Flags
    std::string comment = flag.comment;
    if (!flag.is_category() && flag.is_advanced) {
        if (!comment.empty()) {
            comment += ' ';
        }
        comment += "Changing this setting is not recommended. Hold the Shift key to edit it.";
    }
    // Add
    if (!comment.empty() && ImGui::BeginItemTooltip()) {
        const float max_width = get_max_tooltip_width();
        const float padding = ImGui::GetCursorPosX();
        const float wrap_pos = max_width - padding;
        ImGui::PushTextWrapPos(wrap_pos);
        ImGui::TextUnformatted(comment.c_str());
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}