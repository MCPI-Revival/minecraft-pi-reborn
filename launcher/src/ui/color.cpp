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
ImVec4 Frame::blend_with_primary(const ImVec4 &color) {
    static constexpr ImVec4 primary_color = {1.0f, (69.0f / 255.0f), 0.0f, 1.0f};
    return blend_color(primary_color, color);
}