#pragma once

#include <ostream>

// Cache Version
#define CACHE_VERSION 1

// Load Cache
struct State;
State load_cache();

// Save Cache
void write_cache(std::ostream &stream, const State &state);
void save_cache(const State &state);

// Wipe Cache
void wipe_cache();
