#pragma once

extern "C" {
bool _feature_has(const char *name, int server_default);
}

#define _feature_has_server_disabled (0)
#define _feature_has_server_auto (-1)
#define _feature_has_server_enabled (1)
#define feature_has(name, server_default) _feature_has(name, _feature_has_##server_default)
