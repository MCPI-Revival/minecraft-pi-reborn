#include <vector>
#include <limits>
#include <ranges>

#include <libreborn/util/util.h>
#include <libreborn/config.h>
#include <libreborn/util/exec.h>

#include "configuration.h"
#include "../updater/updater.h"

#include <imgui_stdlib.h>

// Render Distances
static constexpr std::array render_distances = {
    "Far",
    "Normal",
    "Short",
    "Tiny"
};

// Construct
static constexpr int size = 400;
ConfigurationUI::ConfigurationUI(State &state_, bool &save_settings_):
    Frame("Launcher", size, size),
    original_state(state_),
    state(state_),
    save_settings(save_settings_) {}

// Render
int ConfigurationUI::render() {
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
            if (ImGui::BeginTabItem("Servers")) {
                draw_servers();
                ImGui::EndTabItem();
            }
            // About Tab
            if (ImGui::BeginTabItem("About")) {
                draw_about();
                ImGui::EndTabItem();
            }
            ImGui::EndTabBar();
        }
    }
    ImGui::EndChild();
    // Bottom Row
    return draw_bottom();
}

// Bottom Row
int ConfigurationUI::draw_bottom() const {
    // Reset Settings
    const State default_state;
    std::vector<std::tuple<std::string, std::string, const State *>> reset_options = {
        {"Revert", "Last Saved", &original_state},
        {"Reset", "Default", &default_state},
    };
    for (const std::tuple<std::string, std::string, const State *> &option : reset_options) {
        const State &new_state = *std::get<2>(option);
        ImGui::BeginDisabled(state == new_state);
        if (ImGui::Button(std::get<0>(option).c_str())) {
            state = new_state;
        }
        ImGui::SetItemTooltip("Use %s Settings", std::get<1>(option).c_str());
        ImGui::EndDisabled();
        ImGui::SameLine();
    }
    // Right-Align Buttons
    int ret = 0;
    draw_right_aligned_buttons({quit_text, "Launch"}, [&ret](const int id, const bool was_clicked) {
        if (id == 0) {
            // Quit
            if (was_clicked) {
                ret = -1;
            }
            ImGui::SetItemTooltip("Changes Will Not Be Saved!");
        } else if (was_clicked) {
            // Launch
            ret = 1;
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
        scale_format = "Automatic";
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
            if (ImGui::TreeNodeEx(label.c_str(), ImGuiTreeNodeFlags_SpanAvailWidth)) {
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
    draw_right_aligned_buttons({"Clear"}, [&should_clear](__attribute__((unused)) const int id, const bool was_clicked) {
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
static int server_list_address_filter(ImGuiInputTextCallbackData *data) {
    // Lowercase
    constexpr std::pair lower_alpha = {'a', 'z'};
    constexpr std::pair upper_alpha = {'A', 'Z'};
    ImWchar &x = data->EventChar;
    if (x >= upper_alpha.first && x <= upper_alpha.second) {
        x += lower_alpha.first - upper_alpha.first;
    }
    // Check Characters
    return (x >= lower_alpha.first && x <= lower_alpha.second) || x == '.' ? 0 : 1;
}
static int server_list_port_filter(ImGuiInputTextCallbackData *data) {
    // Only Allow Integers
    const ImWchar &x = data->EventChar;
    return x >= '0' && x <= '9' ? 0 : 1;
}
void ConfigurationUI::draw_server_list() const {
    for (std::vector<ServerList::Entry>::size_type i = 0; i < state.servers.entries.size(); ++i) {
        ServerList::Entry &entry = state.servers.entries[i];

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
        ImGui::InputTextWithHint((base_label + address_hint).c_str(), address_hint, &entry.first, ImGuiInputTextFlags_CallbackCharFilter, server_list_address_filter);
        ImGui::PopItemWidth();

        // Port
        ServerList::port_t &port = entry.second;
        std::string port_str = port > 0 ? std::to_string(port) : "";
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

// About
void ConfigurationUI::draw_centered_text(const std::string &str) {
    const float width = ImGui::GetWindowSize().x;
    const float text_width = ImGui::CalcTextSize(str.c_str()).x;
    ImGui::SetCursorPosX((width - text_width) / 2.0f);
    ImGui::Text("%s", str.c_str());
}
void ConfigurationUI::draw_links(const std::vector<std::pair<std::string, std::string>> &links) {
    std::vector<const char *> buttons;
    for (const std::string &text : links | std::views::keys) {
        buttons.push_back(text.c_str());
    }
    draw_right_aligned_buttons(buttons, [&links](const int id, const bool was_clicked) {
        if (was_clicked) {
            open_url(links[id].second);
        }
    }, true);
}
void ConfigurationUI::draw_about() {
    // Text
    draw_centered_text("By " MCPI_AUTHOR);
    draw_centered_text("Version " MCPI_VERSION);
    // Links
    ImGui::Separator();
    draw_links({
        {"Home", MCPI_REPO},
        {"Changelog", MCPI_DOCS_CHANGELOG},
        {"Credits", MCPI_DOCS "CREDITS.md"}
    });
    // Updater
    Updater *updater = Updater::instance;
    if (updater) {
        ImGui::Separator();
        ImGui::BeginDisabled(!updater->can_start());
        draw_right_aligned_buttons({updater->get_status().c_str()}, [&updater](__attribute__((unused)) int id, const bool was_clicked) {
            if (was_clicked) {
                updater->start();
            }
        }, true);
        ImGui::EndDisabled();
    }
}
