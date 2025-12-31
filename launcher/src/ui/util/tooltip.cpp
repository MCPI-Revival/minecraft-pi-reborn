#include "../frame.h"

// No Public API Available
#include <imgui_internal.h>

// Get Maximum Tooltip Width
float Frame::get_max_tooltip_width() {
    ImGuiWindow *window = ImGui::GetCurrentWindowRead();
    const ImRect rect = ImGui::GetPopupAllowedExtentRect(window);
    return rect.GetWidth();
}