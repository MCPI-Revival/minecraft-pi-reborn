#pragma once

#include <functional>
#include <string>

// Patch A Texture Atlas
struct Textures;
class AtlasPatcher {
    // Properties
    typedef std::function<bool(unsigned char &, unsigned char &, unsigned char &, unsigned char &)> Callback;
    const std::string atlas;
    Textures *const textures;

public:
    // Methods
    explicit AtlasPatcher(const std::string &atlas_, Textures *textures_);
    [[nodiscard]] bool patch(int input, int output, const Callback &callback) const;
};