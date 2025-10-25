#include <vector>
#include <string>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "glfw.h"

// Set Window Icon
void _reborn_set_window_icon(GLFWwindow *window) {
    // Get The Process Handle
    const HINSTANCE instance = GetModuleHandleA(nullptr);
    if (!instance) {
        return;
    }

    // Get Icon
    const HICON icon = (HICON) LoadImageA(instance, "IDI_ICON1", IMAGE_ICON, 0, 0, LR_DEFAULTSIZE);
    if (!icon) {
        return;
    }

    // Get Size
    ICONINFO icon_info = {};
    GetIconInfo(icon, &icon_info);
    BITMAP icon_color = {};
    GetObjectA(icon_info.hbmColor, sizeof(icon_color), &icon_color);
    const int width = icon_color.bmWidth;
    const int height = icon_color.bmHeight;
    constexpr int channels = 4;
    const int size = width * height * channels;
    std::vector<unsigned char> pixels(size);

    // Get Pixels
    const HDC hdc = GetDC(nullptr);
    BITMAPINFO bitmap_info = {};
    bitmap_info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bitmap_info.bmiHeader.biWidth = width;
    bitmap_info.bmiHeader.biHeight = -height;  // top-down DIB
    bitmap_info.bmiHeader.biPlanes = 1;
    bitmap_info.bmiHeader.biBitCount = channels * CHAR_BIT;
    bitmap_info.bmiHeader.biCompression = BI_RGB;
    GetDIBits(hdc, icon_info.hbmColor, 0, height, pixels.data(), &bitmap_info, DIB_RGB_COLORS);
    ReleaseDC(nullptr, hdc);

    // Convert BGRA To RGBA
    for (int i = 0; i < size; i += channels) {
        unsigned char &b = pixels.at(i);
        unsigned char &r = pixels.at(i + 2);
        std::swap(b, r);
    }

    // Set Icon
    GLFWimage image;
    image.width = width;
    image.height = height;
    image.pixels = pixels.data();
    glfwSetWindowIcon(window, 1, &image);

    // Free
    DeleteObject(icon_info.hbmColor);
    DeleteObject(icon_info.hbmMask);
    DestroyIcon(icon);
}