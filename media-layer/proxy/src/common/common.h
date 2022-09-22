#pragma once

#include <stdint.h>

#include <libreborn/libreborn.h>

#ifdef __cplusplus
extern "C" {
#endif

#if __BYTE_ORDER != __LITTLE_ENDIAN
#error "Only Little Endian Is Supported"
#endif

#if defined(MEDIA_LAYER_PROXY_SERVER)
#include "../server/server.h"
#elif defined(MEDIA_LAYER_PROXY_CLIENT)
#include "../client/client.h"
#else
#error "Invalid Configuration"
#endif

#define CONNECTED_MSG "Connected"

#define PROXY_INFO(format, ...) RAW_DEBUG(PROXY_LOG_TAG, format, ##__VA_ARGS__);
#define PROXY_ERR(format, ...) { close_connection(); ERR(PROXY_LOG_TAG format, ##__VA_ARGS__); }

// Safely Send/Receive Data From The Connection
__attribute__((visibility("internal"))) void safe_read(void *buf, size_t len);
__attribute__((visibility("internal"))) void safe_write(void *buf, size_t len);
__attribute__((visibility("internal"))) void flush_write_cache();
__attribute__((visibility("internal"))) void void_write_cache();

// Read/Write 32-Bit Integers
__attribute__((visibility("internal"))) uint32_t read_int();
__attribute__((visibility("internal"))) void write_int(uint32_t x);

// Read/Write Bytes
__attribute__((visibility("internal"))) unsigned char read_byte();
__attribute__((visibility("internal"))) void write_byte(unsigned char x);

// Read/Write Floats
__attribute__((visibility("internal"))) float read_float();
__attribute__((visibility("internal"))) void write_float(float x);

// Read/Write Strings
__attribute__((visibility("internal"))) char *read_string(); // Remember To free()
__attribute__((visibility("internal"))) void write_string(const char *str);

// Manipulate Connection
__attribute__((visibility("internal"))) void set_connection(int read, int write);
__attribute__((visibility("internal"))) int get_connection_read();
__attribute__((visibility("internal"))) int get_connection_write();
__attribute__((visibility("internal"))) void close_connection();
__attribute__((visibility("internal"))) int is_connection_open();

// Check State Of Proxy And Exit If Invalid
__attribute__((visibility("internal"))) void _check_proxy_state();

#ifdef __cplusplus
}
#endif
