#pragma once

#include <string>
#include <unordered_map>

// Cache Version
#define CACHE_VERSION 0

// Load Cache
typedef struct {
    std::string username;
    std::string render_distance;
    std::unordered_map<std::string, bool> feature_flags;
} launcher_cache;
extern launcher_cache empty_cache;
launcher_cache load_cache();

// Save Cache
void save_cache();
