#pragma once

#include <vector>

#include <imgui.h>
#include <GLFW/glfw3.h>

// UI Frame
struct Frame {
    Frame(const char *title, int width, int height);
    virtual ~Frame();

    // Prevent Copying
    Frame(const Frame &) = delete;
    Frame &operator=(const Frame &) = delete;

    // Run
    int run();
    virtual int render() = 0;

protected:
    // API For Sub-Classes
    ImFont *monospace = nullptr;
    static float get_frame_width(const char *str);
    struct AlignedButtonConfig {
        bool should_center = false;
        std::vector<bool> disabled = {};
        std::vector<const char *> tooltips = {};
    };
    static int draw_aligned_buttons(const std::vector<const char *> &buttons, const AlignedButtonConfig &config);
    static constexpr const char *quit_text = "Quit";
    static float get_max_tooltip_width();

private:
    // Properties
    GLFWwindow *window = nullptr;

    // Internal Methods
    float get_scale();
    void setup_fonts();
    static void setup_style(float scale);
    static void patch_colors(ImGuiStyle &style);
};