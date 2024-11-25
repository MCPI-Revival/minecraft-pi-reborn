#pragma once

// Cache Version
#define CACHE_VERSION 0

// Load Cache
struct State;
State load_cache();

// Save Cache
void save_cache(const State &state);

// Wipe Cache
void wipe_cache();
