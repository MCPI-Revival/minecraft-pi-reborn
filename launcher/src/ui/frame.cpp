#include <cmath>

#include "frame.h"

#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl2.h>

#include <libreborn/log.h>
#include <libreborn/util/glfw.h>

// Init/Cleanup
Frame::Frame(const char *title, const int width, const int height) {
    // Create Window
    init_glfw();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 1);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    window = create_glfw_window(title, width, height);

    // Load OpenGL
    if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
        ERR("Unable To Load GLAD");
    }

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

    // Setup Font
    setup_fonts();
}
Frame::~Frame() {
    // Shutdown ImGui
    ImGui_ImplOpenGL2_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    // Cleanup GLFW
    cleanup_glfw(window);
}

// Run Loop
int Frame::run() {
    int ret = 0;
    float last_scale = -1;
    while (!glfwWindowShouldClose(window) && ret == 0) {
        glfwPollEvents();
        if (glfwGetWindowAttrib(window, GLFW_ICONIFIED) != 0) {
            ImGui_ImplGlfw_Sleep(10);
            continue;
        }

        // Update Style
        const float scale = ImGui_ImplGlfw_GetContentScaleForWindow(window);
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
        glfwGetWindowSize(window, &width, &height);
        ImGui::SetNextWindowSize({float(width), float(height)});
        if (ImGui::Begin("##Frame", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoScrollWithMouse)) {
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