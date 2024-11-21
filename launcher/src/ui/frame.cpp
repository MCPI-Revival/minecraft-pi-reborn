#include <cmath>

#include "frame.h"

#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl2.h>

#include <libreborn/libreborn.h>

// Init/Cleanup
Frame::Frame(const char *title, const int width, const int height) {
    // Create Window
    init_glfw();
    window = create_glfw_window(title, width, height);
    // V-Sync
    glfwSwapInterval(1);
    // Setup ImGui Context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.IniFilename = nullptr;
    io.LogFilename = nullptr;
    // Setup Platform/Renderer Backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL2_Init();
}
Frame::~Frame() {
    ImGui_ImplOpenGL2_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    cleanup_glfw(window);
}

// Run Loop
int Frame::run() {
    int ret = 0;
    while (!glfwWindowShouldClose(window) && ret == 0) {
        glfwPollEvents();
        // Update Style
        static float last_scale = -1.0f;
        float scale;
        get_glfw_scale(window, &scale, nullptr);
        if (scale != last_scale) {
            last_scale = scale;
            setup_style(scale);
        }
        // Start Frame
        ImGui_ImplOpenGL2_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        // Main Window
        ImGui::SetNextWindowPos({0, 0});
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        ImGui::SetNextWindowSize({float(width), float(height)});
        if (ImGui::Begin("###Frame", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoScrollWithMouse)) {
            ret = render();
        }
        ImGui::End();
        // Render To OpenGL
        ImGui::Render();
        ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }
    return ret;
}

// Style
EMBEDDED_RESOURCE(Roboto_Medium_ttf);
EMBEDDED_RESOURCE(Cousine_Regular_ttf);
void Frame::setup_style(const float scale) {
    // Fonts
    const ImGuiIO &io = ImGui::GetIO();
    io.Fonts->Clear();
    ImFontConfig font_cfg;
    font_cfg.FontDataOwnedByAtlas = false;
    io.Fonts->AddFontFromMemoryTTF(Roboto_Medium_ttf, int(Roboto_Medium_ttf_len), std::floor(20.0f * scale), &font_cfg);
    monospace = io.Fonts->AddFontFromMemoryTTF(Cousine_Regular_ttf, int(Cousine_Regular_ttf_len), std::floor(18.0f * scale), &font_cfg);
    // Style
    ImGuiStyle &style = ImGui::GetStyle();
    style = ImGuiStyle();
    style.WindowBorderSize = 0;
    ImGui::StyleColorsDark(&style);
    style.ScaleAllSizes(scale);
    patch_colors(style);
}
