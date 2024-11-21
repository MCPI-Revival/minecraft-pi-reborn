#pragma once

#include <imgui.h>
#include <string>
#include <GLFW/glfw3.h>

// UI Frame
struct Frame {
    Frame(const char *title, int width, int height);
    virtual ~Frame();
    // Run
    int run();
    virtual int render() = 0;
    // Properties
protected:
    ImFont *monospace = nullptr;
private:
    GLFWwindow *window = nullptr;
    // Internal
    float get_scale();
    void setup_style(float scale);
    static ImVec4 blend_with_primary(const ImVec4 &color);
};