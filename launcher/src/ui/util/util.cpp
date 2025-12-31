#include "../frame.h"

// Get Width Of A Frame (Like A Button)
float Frame::get_frame_width(const char *str) {
    const ImGuiStyle &style = ImGui::GetStyle();
    return ImGui::CalcTextSize(str).x + style.FramePadding.x * 2.0f;
}

// Draw Aligned Buttons
int Frame::draw_aligned_buttons(const std::vector<const char *> &buttons, const AlignedButtonConfig &config) {
    // Calculate Position
    const ImGuiStyle &style = ImGui::GetStyle();
    float width_needed = 0;
    for (const char *text : buttons) {
        if (width_needed > 0) {
            width_needed += style.ItemSpacing.x;
        }
        width_needed += get_frame_width(text);
    }
    float cursor_pos;
    if (config.should_center) {
        cursor_pos = ImGui::GetWindowSize().x - width_needed;
        cursor_pos /= 2.0f;
    } else {
        cursor_pos = ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x - width_needed;
    }
    ImGui::SetCursorPosX(cursor_pos);

    // Draw
    int ret = -1;
    for (std::vector<const char *>::size_type id = 0; id < buttons.size(); id++) {
        if (id > 0) {
            ImGui::SameLine();
        }
        const bool disabled = !config.disabled.empty() && config.disabled.at(id);
        const char *tooltip = !config.tooltips.empty() ? config.tooltips.at(id) : nullptr;
        ImGui::BeginDisabled(disabled);
        if (ImGui::Button(buttons.at(id))) {
            ret = int(id);
        }
        if (tooltip) {
            ImGui::SetItemTooltip("%s", tooltip);
        }
        ImGui::EndDisabled();
    }
    return ret;
}