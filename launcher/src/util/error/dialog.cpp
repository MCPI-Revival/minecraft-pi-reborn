#include <fstream>

#include <libreborn/config.h>
#include <libreborn/util/exec.h>

#include "../../ui/frame.h"

// UI
struct Error final : Frame {
    // Constructor
    explicit Error(const char *str_):
        Frame("Error", 320, 120),
        str(str_) {}

    // Render Function
    int render() override {
        // Text
        ImGui::TextWrapped("%s", str);

        // Fill Space
        ImGui::Dummy(ImVec2(0, ImGui::GetContentRegionAvail().y - ImGui::GetFrameHeightWithSpacing()));

        // Buttons
        int ret = 0;
        const int clicked_button = draw_aligned_buttons({"Help", "OK"}, {});
        if (clicked_button == 0) {
            open_url(reborn_config.docs.base);
        } else if (clicked_button == 1) {
            // Exit
            ret = 1;
        }
        return ret;
    }

    // Properties
    const char *str;
};

// Show Crash Report Dialog
int main(const int argc, char *argv[]) {
    Error ui(argc > 1 ? argv[1] : "");
    ui.run();
    return EXIT_SUCCESS;
}