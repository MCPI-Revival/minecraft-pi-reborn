#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <sys/ioctl.h>

#include "common.h"

// Safely Send/Receive Data From The Connection
#define CHECK_CONNECTION() \
    { \
        _check_proxy_state(); \
        if (!is_connection_open()) { \
            PROXY_ERR("Attempting To Access Closed Connection"); \
        } \
    }
// Buffer Reads
static void *_read_cache = NULL;
__attribute__((destructor)) static void _free_read_cache() {
    if (_read_cache != NULL) {
        free(_read_cache);
    }
}
static size_t _read_cache_size = 0;
static size_t _read_cache_actual_size = 0;
static size_t _read_cache_position = 0;
#define max(a, b) (((a) > (b)) ? (a) : (b))
#define min(a, b) (((a) < (b)) ? (a) : (b))
void safe_read(void *buf, size_t len) {
    // Check Data
    if (buf == NULL) {
        PROXY_ERR("Attempting To Read Into NULL Buffer");
    }
    // Setup
    size_t to_read = len;
    // Copy From Read Buffer
    if (_read_cache != NULL && _read_cache_size > 0) {
        char *read_cache = (void *) (((unsigned char *) _read_cache) + _read_cache_position);
        size_t read_cache_size = _read_cache_size - _read_cache_position;
        if (read_cache_size > 0) {
            size_t to_copy = min(to_read, read_cache_size);
            memcpy(buf, read_cache, to_copy);
            to_read -= to_copy;
            _read_cache_position += to_copy;
        }
    }
    // Check If Done
    if (to_read < 1) {
        return;
    }
    if (_read_cache_position < _read_cache_size) {
        IMPOSSIBLE();
    }
    // Flush Write Cache
    flush_write_cache();
    // Read Remaining Data
    size_t to_read_to_cache = 0;
    while (to_read_to_cache < 1) {
        int bytes_available;
        if (ioctl(get_connection_read(), FIONREAD, &bytes_available) == -1) {
            bytes_available = 0;
        }
        to_read_to_cache = max((size_t) bytes_available, to_read);
    }
    // Resize Buffer
    _read_cache_position = 0;
    _read_cache_size = to_read_to_cache;
    if (_read_cache == NULL) {
        _read_cache_actual_size = _read_cache_size;
        _read_cache = malloc(_read_cache_actual_size);
    } else if (_read_cache_size > _read_cache_actual_size) {
        _read_cache_actual_size = _read_cache_size;
        _read_cache = realloc(_read_cache, _read_cache_actual_size);
    }
    ALLOC_CHECK(_read_cache);
    // Read Into Buffer
    while (to_read_to_cache > 0) {
        CHECK_CONNECTION();
        ssize_t x = read(get_connection_read(), (void *) (((unsigned char *) _read_cache) + (_read_cache_size - to_read_to_cache)), to_read_to_cache);
        if (x == -1 && errno != EINTR) {
            PROXY_ERR("Failed Reading Data To Connection: %s", strerror(errno));
        }
        to_read_to_cache -= x;
    }
    // Copy Remaining Data
    safe_read((void *) (((unsigned char *) buf) + (len - to_read)), to_read);
}
// Buffer Writes
static void *_write_cache = NULL;
__attribute__((destructor)) static void _free_write_cache() {
    if (_write_cache != NULL) {
        free(_write_cache);
    }
}
static size_t _write_cache_size = 0;
static size_t _write_cache_position = 0;
void safe_write(void *buf, size_t len) {
    // Check Data
    if (buf == NULL) {
        PROXY_ERR("Attempting To Send NULL Data");
    }
    // Expand Write Cache If Needed
    size_t needed_size = _write_cache_position + len;
    if (_write_cache == NULL) {
        _write_cache_size = needed_size;
        _write_cache = malloc(_write_cache_size);
    } else if (needed_size > _write_cache_size) {
        _write_cache_size = needed_size;
        _write_cache = realloc(_write_cache, _write_cache_size);
    }
    ALLOC_CHECK(_write_cache);
    // Copy Data
    memcpy((void *) (((unsigned char *) _write_cache) + _write_cache_position), buf, len);
    // Advance Position
    _write_cache_position += len;
}
// Flush Write Cache
void flush_write_cache() {
    // Check Cache
    if (_write_cache == NULL || _write_cache_position < 1) {
        // Nothing To Write
        return;
    }
    // Check Connection
    if (!is_connection_open()) {
        // Connection Closed
        return;
    }
    // Write & Reset
    size_t to_write = _write_cache_position;
    size_t old_write_cache_position = _write_cache_position;
    _write_cache_position = 0;
    while (to_write > 0) {
        CHECK_CONNECTION();
        ssize_t x = write(get_connection_write(), (void *) (((unsigned char *) _write_cache) + (old_write_cache_position - to_write)), to_write);
        if (x == -1 && errno != EINTR) {
            PROXY_ERR("Failed Writing Data To Connection: %s", strerror(errno));
        }
        to_write -= x;
    }
}
void void_write_cache() {
    _write_cache_position = 0;
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
void write_string(const char *str) {
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
    // Flush Write Cache
    flush_write_cache();
    // Close
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
        PROXY_INFO("Connection Closed");
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
