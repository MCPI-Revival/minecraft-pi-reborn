#include "../configuration.h"

#include <imgui_stdlib.h>

// Render Distances
static constexpr std::array render_distances = {
    "Tiny",
    "Short",
    "Normal",
    "Far"
};
int ConfigurationUI::get_render_distance_index() const {
    int render_distance_index = 0;
    for (std::vector<std::string>::size_type i = 0; i < render_distances.size(); i++) {
        if (std::string(render_distances[i]) == state.render_distance) {
            render_distance_index = int(i);
            break;
        }
    }
    return render_distance_index;
}

// Main Tab
void ConfigurationUI::draw_main() const {
    const ImGuiStyle &style = ImGui::GetStyle();
    const char *labels[] = {"Username", "Render Distance", "UI Scale"};

    // Calculate Label Size
    float label_size = 0;
    for (const char *label : labels) {
        label_size = std::max(label_size, ImGui::CalcTextSize(label).x + style.ItemInnerSpacing.x);
    }
    ImGui::PushItemWidth(-label_size);

    // Username
    ImGui::InputText(labels[0], &state.username);

    // Render Distance
    int render_distance_index = get_render_distance_index();
    if (ImGui::SliderInt(labels[1], &render_distance_index, 0, int(render_distances.size()) - 1, state.render_distance.c_str(), ImGuiSliderFlags_NoInput | ImGuiSliderFlags_AlwaysClamp)) {
        state.render_distance = render_distances[render_distance_index];
    }

    // UI Scale
    int gui_scale_int = int(state.gui_scale); // Fractional GUI Scales Are Messy
    const char *scale_format = "%ix";
    if (gui_scale_int <= AUTO_GUI_SCALE) {
        scale_format = "Automatic";
    }
    if (ImGui::SliderInt(labels[2], &gui_scale_int, 0, MAX_GUI_SCALE, scale_format)) {
        state.gui_scale = float(gui_scale_int);
        if (state.gui_scale < AUTO_GUI_SCALE) {
            state.gui_scale = AUTO_GUI_SCALE;
        }
    }

    // Launcher Cache
    ImGui::PopItemWidth();
    ImGui::Checkbox("Save Settings On Launch", &save_settings);
}