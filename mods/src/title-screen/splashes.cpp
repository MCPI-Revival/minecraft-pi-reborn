#include <fstream>
#include <cmath>

#include <libreborn/patch.h>

#include <symbols/minecraft.h>
#include <GLES/gl.h>

#include <mods/misc/misc.h>
#include <mods/title-screen/title-screen.h>

#include "title-screen-internal.h"

// Load
void title_screen_load_splashes(std::vector<std::string> &splashes) {
    std::ifstream stream("data/splashes.txt");
    if (stream.good()) {
        std::string line;
        while (std::getline(stream, line)) {
            if (line.length() > 0) {
                splashes.push_back(line);
            }
        }
        stream.close();
    } else {
        WARN("Unable To Load Splashes");
    }
}

// Position Splash Text
struct SplashLine {
    static constexpr float x_offset = 64;
    float y_factor;
    static constexpr float angle = -20;
    static constexpr float max_scale = 2.0f;
    static constexpr float min_scale = 0.6f;
    static constexpr float padding = (1.0f / 18.0f);
    const StartMenuScreen *screen;
    explicit SplashLine(const StartMenuScreen *screen_, const float y_factor_): y_factor(y_factor_), screen(screen_) {}
    // Get Origin
    [[nodiscard]] float origin_x() const {
        return (float(screen->width) / 2.0f) + x_offset;
    }
    [[nodiscard]] float get_region() const {
        float region = float(screen->start_button.y - version_text_bottom);
        if (region < 0) {
            region = 0;
        }
        return region;
    }
    [[nodiscard]] float origin_y() const {
        return float(version_text_bottom) + (get_region() / y_factor);
    }
    [[nodiscard]] static float angle_rad() {
        return angle * float(M_PI / 180.0f);
    }
    // Find Endpoint
    [[nodiscard]] float from_x(float x) const {
        x -= origin_x();
        return x / std::cos(angle_rad());
    }
    [[nodiscard]] float from_y(float y) const {
        y -= origin_y();
        return y / std::sin(angle_rad());
    }
    [[nodiscard]] float end() const {
        const float end_x = float(screen->width) * (1 - padding);
        const float end_y = float(screen->width) * padding;
        return std::min(from_x(end_x), from_y(end_y));
    }
    // Get Scale
    [[nodiscard]] float get_max_scale(bool &bad_y_factor) const {
        float region = get_region();
        region /= y_factor;
        const float splash_line_height = region * std::cos(angle_rad());
        float scale = splash_line_height / line_height;
        if (scale > max_scale) {
            scale = max_scale;
            bad_y_factor = true;
        }
        if (scale < min_scale) {
            scale = 0;
        }
        return scale;
    }
};

// Add Splashes
static std::string current_splash;
static bool draw_splash(StartMenuScreen *screen, const float y_factor, const bool allow_bad_y_factor) {
    // Position
    const SplashLine line(screen, y_factor);
    const float x = line.origin_x();
    const float y = line.origin_y();
    // Choose Scale
    const float max_width = line.end();
    bool bad_y_factor = false;
    float scale = line.get_max_scale(bad_y_factor);
    if (bad_y_factor && !allow_bad_y_factor) {
        // Try With Another Y-Factor
        return false;
    }
    const int text_width = screen->font->width(current_splash);
    float splash_width = float(text_width) * scale;
    if (splash_width > float(max_width)) {
        const float multiplier = (float(max_width) / splash_width);
        scale *= multiplier;
        splash_width *= multiplier;
    }
    // Position
    media_glPushMatrix();
    media_glTranslatef(x, y, 0.0f);
    // Rotate
    media_glRotatef(SplashLine::angle, 0.0f, 0.0f, 1.0f);
    // Oscillate
    const float timeMS = float(Common::getTimeMs() % 1000) / 1000.0f;
    const float oscillation = (scale / SplashLine::max_scale) * 0.1f;
    scale = scale - Mth::abs(oscillation * Mth::sin(2.0f * float(M_PI) * timeMS));
    // Scale
    media_glTranslatef(splash_width / 2.0f, 0, 0);
    media_glScalef(scale, scale, 1);
    media_glTranslatef(-text_width / 2.0f, 0, 0);
    // Render
    float y_offset = float(-line_height) / 2.0f;
    y_offset += 1; // Make It Look Vertically Centered
    screen->drawString(screen->font, current_splash, 0, y_offset, 0xffff00);
    // Finish
    media_glPopMatrix();
    return true;
}
static void StartMenuScreen_render_injection(StartMenuScreen_render_t original, StartMenuScreen *screen, const int mouse_x, const int mouse_y, const float param_1) {
    // Call Original Method
    original(screen, mouse_x, mouse_y, param_1);

    // Load Splashes
    static std::vector<std::string> splashes;
    static bool splashes_loaded = false;
    if (!splashes_loaded) {
        // Mark As Loaded
        splashes_loaded = true;
        // Load
        title_screen_load_splashes(splashes);
    }

    // Display Splash
    if (!splashes.empty()) {
        // Pick Splash
        if (current_splash.empty()) {
            current_splash = splashes[rand() % splashes.size()];
        }
        // Draw
        float y_factor = 2.0f;
        bool allow_bad_y_factor = false;
        while (true) {
            if (draw_splash(screen, y_factor, allow_bad_y_factor)) {
                break;
            } else {
                y_factor++;
                allow_bad_y_factor = true;
            }
        }
    }
}
// Reset Splash When Screen Is Opened
static void StartMenuScreen_init_injection_splash(StartMenuScreen_init_t original, StartMenuScreen *screen) {
    // Call Original Method
    original(screen);
    // Reset Splash
    current_splash = "";
}

// Init
void _init_splashes() {
    overwrite_calls(StartMenuScreen_render, StartMenuScreen_render_injection);
    overwrite_calls(StartMenuScreen_init, StartMenuScreen_init_injection_splash);
    // Init Random
    srand(time(nullptr));
}