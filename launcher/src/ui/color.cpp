#include "frame.h"

// Calculate Perceived Brightness Of Color
// See: https://en.wikipedia.org/w/index.php?title=Relative_luminance&useskin=vector
static float compute_luma(const ImVec4 &color) {
    return (0.2126f * color.x) + (0.7152f * color.y) + (0.0722f * color.z);
}

// Apply Luma To RGB
static float clamp(const float value) {
    return std::max(0.0f, std::min(value, 1.0f));
}
static ImVec4 apply_luma_to_color(const float target_luma, const ImVec4 &color) {
    const float current_luma = compute_luma(color);
    const float luma_ratio = (current_luma != 0) ? target_luma / current_luma : 0;
    ImVec4 out = color;
    for (float *x : {&out.x, &out.y, &out.z}) {
        *x = clamp(*x * luma_ratio);
    }
    return out;
}

// Blend Color
static ImVec4 blend_color(const ImVec4 &top, const ImVec4 &bottom) {
    const float luma = compute_luma(bottom);
    ImVec4 out = apply_luma_to_color(luma, top);
    out.w = bottom.w;
    return out;
}
static ImVec4 blend_with_primary(const ImVec4 &color) {
    static constexpr ImVec4 primary_color = {1.0f, (69.0f / 255.0f), 0.0f, 1.0f};
    return blend_color(primary_color, color);
}

// Modify Colors
void Frame::patch_colors(ImGuiStyle &style) {
    // Blend Colors
    static int target_colors_blend[] = {
        ImGuiCol_FrameBg,
        ImGuiCol_FrameBgHovered,
        ImGuiCol_FrameBgActive,
        ImGuiCol_TitleBgActive,
        ImGuiCol_CheckMark,
        ImGuiCol_SliderGrab,
        ImGuiCol_SliderGrabActive,
        ImGuiCol_Button,
        ImGuiCol_ButtonHovered,
        ImGuiCol_ButtonActive,
        ImGuiCol_Header,
        ImGuiCol_HeaderHovered,
        ImGuiCol_HeaderActive,
        ImGuiCol_SeparatorHovered,
        ImGuiCol_SeparatorActive,
        ImGuiCol_ResizeGrip,
        ImGuiCol_ResizeGripHovered,
        ImGuiCol_ResizeGripActive,
        ImGuiCol_TabHovered,
        ImGuiCol_Tab,
        ImGuiCol_TabSelected,
        ImGuiCol_TabSelectedOverline,
        ImGuiCol_TabDimmed,
        ImGuiCol_TabDimmedSelected,
        ImGuiCol_TextLink,
        ImGuiCol_TextSelectedBg,
        ImGuiCol_NavCursor
    };
    for (const int target_color : target_colors_blend) {
        ImVec4 &color = style.Colors[target_color];
        color = blend_with_primary(color);
    }
    // Remove Blue Accent From Colors
    static int target_colors_modify[] = {
        ImGuiCol_Separator,
        ImGuiCol_Border,
        ImGuiCol_TableHeaderBg,
        ImGuiCol_TableBorderStrong,
        ImGuiCol_TableBorderLight
    };
    for (const int target_color : target_colors_modify) {
        ImVec4 &color = style.Colors[target_color];
        color.y = color.z = color.x;
    }
}