#include <errno.h>
#include <string.h>
#include <unistd.h>

#include "common.h"

// Safely Send/Receive Data From The Connection
#define CHECK_CONNECTION() \
    { \
        _check_proxy_state(); \
        if (!is_connection_open()) { \
            PROXY_ERR("%s", "Attempting To Access Closed Connection"); \
        } else { \
            _check_proxy_state(); \
        } \
    }
void safe_read(void *buf, size_t len) {
    if (buf == NULL) {
        PROXY_ERR("%s", "Attempting To Read Into NULL Buffer");
    }
    size_t to_read = len;
    while (to_read > 0) {
        CHECK_CONNECTION();
        ssize_t x = read(get_connection_read(), buf + (len - to_read), to_read);
        if (x == -1 && errno != EINTR) {
            PROXY_ERR("Failed Reading Data To Connection: %s", strerror(errno));
        }
        to_read -= x;
    }
}
// Buffer Writes
void safe_write(void *buf, size_t len) {
    if (buf == NULL) {
        PROXY_ERR("%s", "Attempting To Send NULL Data");
    }
    size_t to_write = len;
    while (to_write > 0) {
        CHECK_CONNECTION();
        ssize_t x = write(get_connection_write(), buf + (len - to_write), to_write);
        if (x == -1 && errno != EINTR) {
            PROXY_ERR("Failed Writing Data To Connection: %s", strerror(errno));
        }
        to_write -= x;
    }
}

// Read/Write 32-Bit Integers
uint32_t read_int() {
    uint32_t ret = 0;
    safe_read((void *) &ret, sizeof (ret));
    return ret;
}
void write_int(uint32_t x) {
    safe_write((void *) &x, sizeof (x));
}

// Read/Write Floats
float read_float() {
    float ret = 0;
    safe_read((void *) &ret, sizeof (ret));
    return ret;
}
void write_float(float x) {
    safe_write((void *) &x, sizeof (x));
}

// Read/Write Bytes
unsigned char read_byte() {
    unsigned char ret = 0;
    safe_read((void *) &ret, sizeof (ret));
    return ret;
}
void write_byte(unsigned char x) {
    safe_write((void *) &x, sizeof (x));
}

// Read/Write Strings
char *read_string() {
    // Check NULL
    unsigned char is_null = read_byte();
    if (is_null) {
        return NULL;
    }
    // Allocate String
    unsigned char length = read_byte();
    char *str = malloc((size_t) length + 1);
    // Read String
    safe_read((void *) str, length);
    // Add Terminator
    str[length] = '\0';
    // Return String
    return strdup(str);
}
#define MAX_STRING_SIZE 256
void write_string(char *str) {
    unsigned char is_null = str == NULL;
    write_byte(is_null);
    if (!is_null) {
        int length = strlen(str);
        if (length > MAX_STRING_SIZE) {
            PROXY_ERR("Unable To Write String To Connection: Larger Than %i Bytes", MAX_STRING_SIZE);
        }
        write_byte((unsigned char) length);
        safe_write((void *) str, length);
    }
}

// Close Connection
void close_connection() {
    int state_changed = 0;
    if (get_connection_read() != -1) {
        close(get_connection_read());
        state_changed = 1;
    }
    if (get_connection_write() != -1) {
        close(get_connection_write());
        state_changed = 1;
    }
    set_connection(-1, -1);
    if (state_changed) {
        PROXY_INFO("%s", "Connection Closed");
    }
}
// Check If Connection Is Open
int is_connection_open() {
    return get_connection_read() != -1 && get_connection_write() != -1;
}
// Pipe
static int _read = -1;
static int _write = -1;
// Set Pipe
void set_connection(int read, int write) {
    _read = read;
    _write = write;
}
// Get Pipe
int get_connection_read() {
    return _read;
}
int get_connection_write() {
    return _write;
}
