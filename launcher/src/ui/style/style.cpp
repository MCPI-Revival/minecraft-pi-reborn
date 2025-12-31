#include <libreborn/util/util.h>

#include "../frame.h"

// Style
void Frame::setup_style(const float scale) {
    // Reset Style
    ImGuiStyle &style = ImGui::GetStyle();
    style = ImGuiStyle();
    // Customize Style
    style.WindowBorderSize = 0;
    ImGui::StyleColorsDark(&style);
    patch_colors(style);
    // Scale Style
    style.ScaleAllSizes(scale);
    style.FontScaleDpi = scale;
}

// Fonts
EMBEDDED_RESOURCE(Roboto_Medium_ttf);
EMBEDDED_RESOURCE(Cousine_Regular_ttf);
void Frame::setup_fonts() {
    const ImGuiIO &io = ImGui::GetIO();
    io.Fonts->Clear();
    ImFontConfig font_cfg;
    font_cfg.FontDataOwnedByAtlas = false;
    io.Fonts->AddFontFromMemoryTTF(Roboto_Medium_ttf, int(Roboto_Medium_ttf_len), 20.0f, &font_cfg);
    monospace = io.Fonts->AddFontFromMemoryTTF(Cousine_Regular_ttf, int(Cousine_Regular_ttf_len), 18.0f, &font_cfg);
}