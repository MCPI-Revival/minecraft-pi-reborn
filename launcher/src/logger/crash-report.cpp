#include <fstream>

#include <libreborn/util/util.h>
#include <libreborn/config.h>
#include <libreborn/util/exec.h>

#include "../ui/frame.h"

// UI
struct CrashReport final : Frame {
    // Constructor
    CrashReport(const char *filename, const char *logs_dir_):
        Frame("Crash Report", 640, 480, true),
        first_render(true),
        logs_dir(logs_dir_)
    {
        // Open File
        if (filename) {
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
    }
    // Render Function
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
            ImGui::PushFont(monospace, monospace->LegacySize);
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
        ImGui::BeginDisabled(logs_dir == nullptr);
        if (ImGui::Button("View All Logs")) {
            open_url("file://" + std::string(logs_dir));
        }
        ImGui::EndDisabled();
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
    // Properties
    bool first_render;
    const char *logs_dir;
    std::string log;
};

// Show Crash Report Dialog
int main(const int argc, char *argv[]) {
    const bool has_file = argc > 1;
    const bool has_dir = argc > 2;
    CrashReport ui(has_file ? argv[1] : nullptr, has_dir ? argv[2] : nullptr);
    ui.run();
    return EXIT_SUCCESS;
}