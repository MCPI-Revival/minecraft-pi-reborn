#include "../glfw.h"

// Set ID Globally
void _reborn_set_app_id_global(const std::string &id) {
    glfwWindowHintString(GLFW_X11_CLASS_NAME, id.c_str());
    glfwWindowHintString(GLFW_WAYLAND_APP_ID, id.c_str());
}