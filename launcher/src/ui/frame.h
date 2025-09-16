#pragma once

#include <vector>
#include <functional>

#include <imgui.h>
#include <GLFW/glfw3.h>

// UI Frame
struct Frame {
    Frame(const char *title, int width, int height, bool block_vsync = false);
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
    static void draw_right_aligned_buttons(const std::vector<const char *> &buttons, const std::function<void(int, bool)> &callback, bool should_actually_center = false);
    static constexpr const char *quit_text = "Quit";
private:
    // Properties
    GLFWwindow *window = nullptr;
    // Internal Methods
    float get_scale();
    void setup_fonts();
    static void setup_style(float scale);
    static void patch_colors(ImGuiStyle &style);
};