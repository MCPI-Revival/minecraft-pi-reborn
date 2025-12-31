#include "../frame.h"

// Blend Color
template <void (*func)(float, float, float, float &, float &, float &)>
static void convert_color(ImVec4 &color) {
    func(color.x, color.y, color.z, color.x, color.y, color.z);
}
static ImVec4 blend_with_primary(ImVec4 color) {
    // Convert New Primary Color To HSV
    ImVec4 primary_color = {1.0f, (69.0f / 255.0f), 0.0f, 0.0f};
    convert_color<ImGui::ColorConvertRGBtoHSV>(primary_color);
    // Convert Input To HSV
    convert_color<ImGui::ColorConvertRGBtoHSV>(color);
    // Modify Color
    color.x = primary_color.x;
    color.y = primary_color.y;
    // Output
    convert_color<ImGui::ColorConvertHSVtoRGB>(color);
    return color;
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