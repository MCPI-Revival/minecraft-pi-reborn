#pragma once

#ifdef __cplusplus
extern "C" {
#endif

int _feature_has(const char *name);

#ifdef MCPI_SERVER_MODE
#define _feature_has__server_defaul_is_server_disabled(name) 0
#define _feature_has__server_defaul_is_server_auto(name) _feature_has(name)
#define _feature_has__server_defaul_is_server_enabled(name) 1
#define feature_has(name, server_default) _feature_has__server_defaul_is_##server_default(name)
#else
#define feature_has(name, server_default) _feature_has(name)
#endif

#ifdef __cplusplus
}
#endif
