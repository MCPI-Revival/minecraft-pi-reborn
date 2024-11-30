#include <vector>
#include <limits>
#include <cmath>

#include <libreborn/util.h>

#include "configuration.h"

#include <imgui_stdlib.h>

// Render Distances
static constexpr std::array render_distances = {
    "Far",
    "Normal",
    "Short",
    "Tiny"
};

// Tooltips/Text
static const char *revert_text = "Revert";
static const char *revert_tooltip_text = "Last Saved";
static std::string make_tooltip(const std::string &text, const std::string &type) {
    return "Use " + text + ' ' + type;
}

// Construct
static constexpr int size = 400;
ConfigurationUI::ConfigurationUI(State &state_, bool &save_settings_):
    Frame("Launcher", size, size),
    original_state(state_),
    state(state_),
    save_settings(save_settings_) {}

// Render
int ConfigurationUI::render() {
    bool on_servers_tab = false;
    if (ImGui::BeginChild("Main", ImVec2(0, -ImGui::GetFrameHeightWithSpacing() /* Leave Room For Bottom Row */), ImGuiChildFlags_None, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse)) {
        // Tabs
        if (ImGui::BeginTabBar("TabBar")) {
            // Main Tab
            if (ImGui::BeginTabItem("General")) {
                draw_main();
                ImGui::EndTabItem();
            }
            // Advanced Tab
            if (ImGui::BeginTabItem("Advanced")) {
                draw_advanced();
                ImGui::EndTabItem();
            }
            // Servers Tab
            if (ImGui::BeginTabItem("Servers", nullptr, are_servers_unsaved() ? ImGuiTabItemFlags_UnsavedDocument : ImGuiTabItemFlags_None)) {
                draw_servers();
                ImGui::EndTabItem();
                on_servers_tab = true;
            }
            ImGui::EndTabBar();
        }
    }
    ImGui::EndChild();
    // Bottom Row
    return draw_bottom(on_servers_tab);
}

// Bottom Row
int ConfigurationUI::draw_bottom(const bool hide_reset_revert) const {
    // Reset Settings
    if (!hide_reset_revert) {
        const State default_state;
        constexpr const char *tooltip_type = "Settings";
        std::vector<std::tuple<std::string, std::string, const State *>> reset_options = {
            {revert_text, make_tooltip(revert_tooltip_text, tooltip_type), &original_state},
            {"Reset", make_tooltip("Default", tooltip_type), &default_state},
        };
        for (const std::tuple<std::string, std::string, const State *> &option : reset_options) {
            const State &new_state = *std::get<2>(option);
            ImGui::BeginDisabled(state == new_state);
            if (ImGui::Button(std::get<0>(option).c_str())) {
                state = new_state;
            }
            ImGui::SetItemTooltip("%s", std::get<1>(option).c_str());
            ImGui::EndDisabled();
            ImGui::SameLine();
        }
    }
    // Right-Align Buttons
    int ret = 0;
    bool unsaved_servers = are_servers_unsaved();
    draw_right_aligned_buttons({quit_text, "Launch"}, [&ret, unsaved_servers](const int id, const bool was_clicked) {
        if (id == 0) {
            // Quit
            if (was_clicked) {
                ret = -1;
            }
            ImGui::SetItemTooltip("Changes Will Not Be Saved!");
            // Disable Launch if Server List Is Unsaved
            if (unsaved_servers) {
                ImGui::BeginDisabled();
            }
        } else if (id == 1) {
            // Launch
            if (unsaved_servers) {
                ImGui::SetItemTooltip("Server List Is Unsaved");
                ImGui::EndDisabled();
            }
            if (was_clicked) {
                ret = 1;
            }
        }
    });
    // Return
    return ret;
}

// Main Tab
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
    if (ImGui::Combo(labels[1], &render_distance_index, render_distances.data(), int(render_distances.size()))) {
        state.render_distance = render_distances[render_distance_index];
    }
    // UI Scale
    int gui_scale_int = int(state.gui_scale); // Fractional GUI Scales Are Messy
    std::string scale_format = "%ix";
    if (gui_scale_int <= AUTO_GUI_SCALE) {
        scale_format = "Auto";
    }
    if (ImGui::SliderInt(labels[2], &gui_scale_int, 0, 8, scale_format.c_str())) {
        state.gui_scale = float(gui_scale_int);
        if (state.gui_scale < AUTO_GUI_SCALE) {
            state.gui_scale = AUTO_GUI_SCALE;
        }
    }
    ImGui::PopItemWidth();
    // Launcher Cache
    ImGui::Checkbox("Save Settings On Launch", &save_settings);
}

// Advanced Tab
static std::string get_label_for_flag_node(const FlagNode &node) {
    return node.name + "##FlagNode" + std::to_string(node.id);
}
void ConfigurationUI::draw_advanced() const {
    if (ImGui::BeginChild("Features", ImVec2(0, 0), ImGuiChildFlags_Borders, ImGuiWindowFlags_HorizontalScrollbar)) {
        // Categories
        for (FlagNode &category : state.flags.root.children) {
            const std::string label = get_label_for_flag_node(category);
            if (ImGui::CollapsingHeader(label.c_str())) {
                draw_category(category);
            }
        }
    }
    ImGui::EndChild();
}
void ConfigurationUI::draw_category(FlagNode &category) {
    for (FlagNode &child : category.children) {
        const std::string label = get_label_for_flag_node(child);
        if (!child.children.empty()) {
            // Sub-Category
            if (ImGui::TreeNode(label.c_str())) {
                draw_category(child);
                ImGui::TreePop();
            }
        } else {
            // Flag
            ImGui::Checkbox(label.c_str(), &child.value);
        }
    }
}

// Servers
bool ConfigurationUI::are_servers_unsaved() const {
    return servers.to_string() != last_saved_servers.to_string();
}
void ConfigurationUI::draw_servers() {
    // Add/Clear
    bool scroll_to_bottom = false;
    if (ImGui::Button("Add")) {
        servers.entries.emplace_back("", DEFAULT_MULTIPLAYER_PORT);
        scroll_to_bottom = true;
    }
    ImGui::SameLine();
    ImGui::BeginDisabled(servers.entries.empty());
    if (ImGui::Button("Clear")) {
        servers.entries.clear();
    }
    ImGui::EndDisabled();
    ImGui::SameLine();
    // Revert/Save
    int clicked_button = -1;
    ImGui::BeginDisabled(!are_servers_unsaved());
    draw_right_aligned_buttons({revert_text, "Save"}, [&clicked_button](const int id, const bool was_clicked) {
        if (id == 0) {
            ImGui::SetItemTooltip("%s", make_tooltip(revert_tooltip_text, "Server List").c_str());
        }
        if (was_clicked) {
            clicked_button = id;
        }
    });
    ImGui::EndDisabled();
    if (clicked_button == 1) {
        // Save
        servers.save();
        last_saved_servers = servers;
    } else if (clicked_button == 0) {
        // Revert To Last Saved Server List
        servers = last_saved_servers;
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
void ConfigurationUI::draw_server_list() {
    for (std::vector<ServerList::Entry>::size_type i = 0; i < servers.entries.size(); ++i) {
        ServerList::Entry &entry = servers.entries[i];

        // Calculate Item Widths
        const ImGuiStyle &style = ImGui::GetStyle();
        const std::string port_width_text = std::to_string(int(std::numeric_limits<ServerList::port_t>::max()) * 2); // Should Comfortably Fit All Port Numbers
        const std::string delete_text = "Delete";
        const float port_width = get_frame_width(port_width_text.c_str());
        const float width_needed = (style.ItemSpacing.x * 2.0f) + port_width + get_frame_width(delete_text.c_str());

        // Labels
        const std::string base_label = "##ServerEntry" + std::to_string(i);

        // Hints
        const char *address_hint = "Address";
        const char *port_hint = "Port";

        // Address
        ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x - width_needed);
        ImGui::InputTextWithHint((base_label + address_hint).c_str(), address_hint, &entry.first, ImGuiInputTextFlags_CharsNoBlank);
        ImGui::PopItemWidth();

        // Port
        ServerList::port_t &port = entry.second;
        std::string port_str = port > 0 ? std::to_string(port) : "";
        ImGui::SameLine();
        ImGui::PushItemWidth(port_width);
        if (ImGui::InputTextWithHint((base_label + port_hint).c_str(), port_hint, &port_str, ImGuiInputTextFlags_CharsDecimal | ImGuiInputTextFlags_NoHorizontalScroll)) {
            port = ServerList::parse_port(port_str);
        }
        ImGui::PopItemWidth();

        // Delete
        ImGui::SameLine();
        if (ImGui::Button((delete_text + base_label).c_str())) {
            servers.entries.erase(servers.entries.begin() + int(i));
            i--;
        }
    }
}