#include <ranges>

#include <libreborn/config.h>
#include <libreborn/util/exec.h>

#include "../configuration.h"
#include "../../updater/updater.h"
#include "../../util/util.h"

// Utility Functions
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

// About
void ConfigurationUI::draw_about() {
    // Text
    draw_centered_text("By " MCPI_AUTHOR);
    draw_centered_text("Version " MCPI_VERSION);

    // Links
    ImGui::Separator();
    draw_links({
        {"Home", MCPI_REPO},
        {"Changelog", MCPI_CHANGELOG},
        {"Credits", MCPI_DOCS "CREDITS.md"}
    });

    // Desktop File
    ImGui::Separator();
    ImGui::BeginDisabled(is_desktop_file_installed());
    draw_right_aligned_buttons({"Create Desktop Entry"}, [](__attribute__((unused)) int id, const bool was_clicked) {
        if (was_clicked) {
            copy_desktop_file();
        }
    }, true);
    ImGui::EndDisabled();

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