#include <fstream>

#include <libreborn/libreborn.h>

#include "logger.h"
#include "../ui/frame.h"

// UI
struct CrashReport final : Frame {
    explicit CrashReport(const char *filename): Frame("Crash Report", 640, 480) {
        // Open File
        std::ifstream stream(filename, std::ios_base::binary | std::ios_base::ate);
        if (stream) {
            // Read File
            const std::streamoff size = stream.tellg();
            stream.seekg(0, std::ifstream::beg);
            log.resize(size);
            stream.read(log.data(), size);
            // Close File
            stream.close();
        }
    }
    bool first_render = true;
    int render() override {
        // Text
        ImGui::TextWrapped("%s", MCPI_APP_TITLE " has crashed!");
        ImGui::Spacing();
        ImGui::TextWrapped("Need help? Consider asking on the Discord server!");
        ImGui::Spacing();
        ImGui::TextWrapped("If you believe this is a problem with " MCPI_APP_TITLE " itself, please upload this crash report to the #bugs Discord channel.");
        // Log
        if (ImGui::BeginChild("Log", ImVec2(0, -ImGui::GetFrameHeightWithSpacing() /* Leave Room For Bottom Row */), ImGuiChildFlags_Borders, ImGuiWindowFlags_HorizontalScrollbar)) {
            ImGui::PushFont(monospace);
            ImGui::TextUnformatted(log.data(), log.data() + log.size());
            ImGui::PopFont();
            if (first_render) {
                ImGui::SetScrollHereY(1.0f);
                first_render = false;
            }
        }
        ImGui::EndChild();
        // Buttons
        if (ImGui::Button("Join Discord")) {
            open_url(MCPI_DISCORD_INVITE);
        }
        ImGui::SameLine();
        if (ImGui::Button("View All Logs")) {
            open_url("file://" + get_logs_folder());
        }
        ImGui::SameLine();
        // Right-Aligned
        int ret = 0;
        const std::string &log_ref = log;
        draw_right_aligned_buttons({"Copy", "Quit"}, [&ret, &log_ref](const int id, const bool was_clicked) {
            if (was_clicked) {
                if (id == 0) {
                    // Copy Log
                    ImGui::SetClipboardText(log_ref.c_str());
                } else {
                    // Exit
                    ret = 1;
                }
            }
        });
        return ret;
    }
    std::string log;
};

// Show Crash Report Dialog
void show_report(const char *log_filename) {
    // Fork
    const pid_t pid = fork();
    if (pid == 0) {
        // Child
        setsid();
        ALLOC_CHECK(freopen("/dev/null", "w", stdout));
        ALLOC_CHECK(freopen("/dev/null", "w", stderr));
        ALLOC_CHECK(freopen("/dev/null", "r", stdin));
        CrashReport ui(log_filename);
        ui.run();
        exit(EXIT_SUCCESS);
    }
}