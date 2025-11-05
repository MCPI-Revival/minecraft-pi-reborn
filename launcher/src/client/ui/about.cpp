#include <ranges>

#include <libreborn/config.h>
#include <libreborn/util/exec.h>

#include "../configuration.h"
#include "../../updater/updater.h"
#include "../../install/install.h"

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
    const int ret = draw_aligned_buttons(buttons, {
        .should_center = true
    });
    if (ret >= 0) {
        open_url(links.at(ret).second);
    }
}

// About
void ConfigurationUI::draw_about() {
    // Text
    draw_centered_text(std::string("By ") + reborn_config.general.author);
    draw_centered_text(std::string("Version ") + reborn_config.general.version);

    // Links
    ImGui::Separator();
    draw_links({
        {"Home", reborn_config.extra.repo_url},
        {"Changelog", reborn_config.docs.changelog},
        {"Credits", std::string(reborn_config.docs.base) + "CREDITS.md"}
    });

    // Desktop File
    if (!reborn_is_using_package_manager()) {
        ImGui::Separator();
        const bool is_desktop_entry_installed = is_desktop_file_installed();
        const std::string desktop_entry_button = std::string(is_desktop_entry_installed ? "Remove" : "Create") + " Desktop Entry";
        const int ret = draw_aligned_buttons({desktop_entry_button.c_str()}, {
            .should_center = true
        });
        if (ret == 0) {
            if (is_desktop_entry_installed) {
                remove_desktop_file();
            } else {
                copy_desktop_file();
            }
        }
    }

    // Updater
    if (!reborn_is_using_package_manager()) {
        ImGui::Separator();
        const std::string status = updater.get_status();
        const int ret = draw_aligned_buttons({status.c_str()}, {
            .should_center = true,
            .disabled = {!updater.can_start()}
        });
        if (ret == 0) {
            updater.start();
        }
    }
}