#include <fstream>

#include <libreborn/util/util.h>
#include <libreborn/config.h>
#include <libreborn/util/exec.h>

#include "logger.h"
#include "../ui/frame.h"

// UI
struct CrashReport final : Frame {
    explicit CrashReport(const char *filename): Frame("Crash Report", 640, 480, true) {
        // Open File
        std::ifstream stream(filename, std::ios::binary | std::ios::ate);
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
        const char *title = reborn_config.app.title;
        ImGui::TextWrapped("%s has crashed!", title);
        ImGui::Spacing();
        ImGui::TextWrapped("Need help? Consider asking on the Discord server!");
        ImGui::Spacing();
        ImGui::TextWrapped("If you believe this is a problem with %s itself, please upload this crash report to the #bugs Discord channel.", title);
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
            open_url(reborn_config.extra.discord_invite);
        }
        ImGui::SameLine();
        if (ImGui::Button("View All Logs")) {
            open_url("file://" + get_logs_folder());
        }
        ImGui::SameLine();
        // Right-Aligned
        int ret = 0;
        const std::string &log_ref = log;
        draw_right_aligned_buttons({"Copy", quit_text}, [&ret, &log_ref](const int id, const bool was_clicked) {
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
static void redirect_file(FILE *file, const char *mode) {
    const FILE *ret = freopen("/dev/null", mode, file);
    if (!ret) {
        IMPOSSIBLE();
    }
}
void show_report(const char *log_filename) {
    // Fork
    const pid_t pid = fork();
    if (pid == 0) {
        // Child
        setsid();
        redirect_file(stdout, "w");
        redirect_file(stderr, "w");
        redirect_file(stdin, "r");
        CrashReport ui(log_filename);
        ui.run();
        exit(EXIT_SUCCESS);
    }
}