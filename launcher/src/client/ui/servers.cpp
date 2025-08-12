#include <limits>

#include <libreborn/util/util.h>
#include <libreborn/util/string.h>

#include "../configuration.h"

#include <imgui_stdlib.h>

// Servers
void ConfigurationUI::draw_servers() const {
    // Add
    bool scroll_to_bottom = false;
    if (ImGui::Button("Add")) {
        state.servers.entries.emplace_back("", DEFAULT_MULTIPLAYER_PORT);
        scroll_to_bottom = true;
    }
    ImGui::SameLine();

    // Clear
    bool should_clear = false;
    ImGui::BeginDisabled(state.servers.entries.empty());
    draw_right_aligned_buttons({"Clear"}, [&should_clear](MCPI_UNUSED const int id, const bool was_clicked) {
        should_clear = was_clicked;
    });
    ImGui::EndDisabled();
    if (should_clear) {
        state.servers.entries.clear();
    }

    // List
    if (ImGui::BeginChild("ServerList", ImVec2(0, 0), ImGuiChildFlags_Borders)) {
        draw_server_list();
        if (scroll_to_bottom) {
            ImGui::SetScrollHereY(1.0f);
        }
    }
    ImGui::EndChild();
}

// Text Filters
static bool is_numeric(const ImWchar x) {
    return x >= '0' && x <= '9';
}
static int server_list_address_filter(ImGuiInputTextCallbackData *data) {
    // Lowercase
    constexpr std::pair lower_alpha = {'a', 'z'};
    constexpr std::pair upper_alpha = {'A', 'Z'};
    ImWchar &x = data->EventChar;
    if (x >= upper_alpha.first && x <= upper_alpha.second) {
        x += lower_alpha.first - upper_alpha.first;
    }
    if (x >= lower_alpha.first && x <= lower_alpha.second) {
        return 0;
    }
    // Numbers
    if (is_numeric(x)) {
        return 0;
    }
    // Other Allowed Characters
    if (x == '.' || x == '-') {
        return 0;
    }
    // Not Allowed
    return 1;
}
static int server_list_port_filter(ImGuiInputTextCallbackData *data) {
    // Only Allow Integers
    const ImWchar &x = data->EventChar;
    return is_numeric(x) ? 0 : 1;
}

// Draw List
void ConfigurationUI::draw_server_list() const {
    for (std::vector<ServerList::Entry>::size_type i = 0; i < state.servers.entries.size(); ++i) {
        ServerList::Entry &entry = state.servers.entries[i];

        // Calculate Item Widths
        const ImGuiStyle &style = ImGui::GetStyle();
        const std::string port_width_text = safe_to_string(int(std::numeric_limits<ServerList::port_t>::max()) * 2); // Should Comfortably Fit All Port Numbers
        const std::string delete_text = "Delete";
        const float port_width = get_frame_width(port_width_text.c_str());
        const float width_needed = (style.ItemSpacing.x * 2.0f) + port_width + get_frame_width(delete_text.c_str());

        // Labels
        const std::string base_label = "##ServerEntry" + safe_to_string(i);

        // Hints
        const char *address_hint = "Address";
        const char *port_hint = "Port";

        // Address
        ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x - width_needed);
        ImGui::InputTextWithHint((base_label + address_hint).c_str(), address_hint, &entry.first, ImGuiInputTextFlags_CallbackCharFilter, server_list_address_filter);
        ImGui::PopItemWidth();

        // Port
        ServerList::port_t &port = entry.second;
        std::string port_str = port > 0 ? safe_to_string(port) : "";
        ImGui::SameLine();
        ImGui::PushItemWidth(port_width);
        if (ImGui::InputTextWithHint((base_label + port_hint).c_str(), port_hint, &port_str, ImGuiInputTextFlags_CallbackCharFilter | ImGuiInputTextFlags_NoHorizontalScroll, server_list_port_filter)) {
            port = ServerList::parse_port(port_str);
        }
        ImGui::PopItemWidth();

        // Delete
        ImGui::SameLine();
        if (ImGui::Button((delete_text + base_label).c_str())) {
            state.servers.entries.erase(state.servers.entries.begin() + int(i));
            i--;
        }
    }
}