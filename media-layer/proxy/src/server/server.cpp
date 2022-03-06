#include <cerrno>
#include <cstring>
#include <unistd.h>
#include <pthread.h>
#include <csignal>
#include <sys/wait.h>
#include <fcntl.h>

#include <string>
#include <unordered_map>
#include <fstream>

#include <media-layer/core.h>

#include "../common/common.h"

// Track Client State
static int _client_is_alive = 0;
static int _client_status = 0;
static void update_client_state(int is_alive, int status) {
    _client_is_alive = is_alive;
    _client_status = status;
}
// Check State Of Proxy And Exit If Invalid
void _check_proxy_state() {
    // Check Client State
    if (!_client_is_alive) {
        void_write_cache(); // Child Is Dead, No Reason To Send A Dead Process Data
        if (WIFEXITED(_client_status)) {
            PROXY_ERR("Client Terminated: Exit Code: %i", WEXITSTATUS(_client_status));
        } else if (WIFSIGNALED(_client_status)) {
            PROXY_ERR("Client Terminated: Signal: %i%s", WTERMSIG(_client_status), WCOREDUMP(_client_status) ? " (Core Dumped)" : "");
        } else {
            PROXY_ERR("%s", "Client Terminated");
        }
    }
}

// Start Proxy Client
static pid_t _client_pid;
static void sigchld_handler(__attribute__((unused)) int sig) {
    // Track
    int status;

    // Reap
    int saved_errno = errno;
    // Only waitpid() Proxy Client, Other Sub-Processes Are Handled By pclose()
    if (waitpid(_client_pid, &status, WNOHANG) == _client_pid) {
        // Handle Client Death
        update_client_state(0, status);
    }
    errno = saved_errno;
}
static void start_media_layer_proxy_client(int read, int write) {
    // Reap Children
    struct sigaction sa;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_NOCLDSTOP;
    sa.sa_handler = &sigchld_handler;
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        PROXY_ERR("Unable To Install Signal Handler: %s", strerror(errno));
    }

    // Fork And Start
    pid_t ret = fork();
    if (ret == -1) {
        PROXY_ERR("Unable To Launch Client: %s", strerror(errno));
    } else if (ret == 0) {
        // Child Process

        // Prepare Environment
        RESET_ENVIRONMENTAL_VARIABLE("LD_LIBRARY_PATH");
        RESET_ENVIRONMENTAL_VARIABLE("LD_PRELOAD");

        // Prepare Arguments
        char *read_str = NULL;
        safe_asprintf(&read_str, "%i", read);
        char *write_str = NULL;
        safe_asprintf(&write_str, "%i", write);
        char *argv[] = {NULL /* Updated By safe_execvpe() */, read_str, write_str, NULL};

        // Run
        safe_execvpe("media-layer-proxy-client", argv, environ);
    } else {
        // Parent Process
        _client_pid = ret;
    }
    update_client_state(1, 0);
}

// Start Server
static int loaded = 0;
__attribute__((constructor)) void media_ensure_loaded() {
    if (!loaded) {
        loaded = 1;

        // Log
        PROXY_INFO("%s", "Starting...");

        // Create Connection
        int server_to_client_pipe[2];
        safe_pipe2(server_to_client_pipe, 0);
        int client_to_server_pipe[2];
        safe_pipe2(client_to_server_pipe, 0);
        // Set Connection
        set_connection(client_to_server_pipe[0], server_to_client_pipe[1]);

        // Start Client
        start_media_layer_proxy_client(server_to_client_pipe[0], client_to_server_pipe[1]);

        // Wait For Connection Message
        char *str = read_string();
        if (strcmp(str, CONNECTED_MSG) == 0) {
            PROXY_INFO("%s", "Connected");
        } else {
            PROXY_ERR("%s", "Unable To Connect");
        }
        // Free
        free(str);
    }
}

// Assign Unique ID To Function
static std::unordered_map<std::string, unsigned char> &get_unique_ids() {
    static std::unordered_map<std::string, unsigned char> unique_ids;
    return unique_ids;
}
void _assign_unique_id(const char *name, unsigned char id) {
    get_unique_ids()[name] = id;
}
unsigned char _get_unique_id(const char *name) {
    return get_unique_ids()[name]; // Assume ID Exists
}

// The Proxy Is Single-Threaded
static pthread_mutex_t proxy_mutex = PTHREAD_MUTEX_INITIALIZER;
void _start_proxy_call(unsigned char call_id) {
    // Lock Proxy
    pthread_mutex_lock(&proxy_mutex);
    write_byte(call_id);
}
void end_proxy_call() {
    // Flush Write Cache
    flush_write_cache();
    // Release Proxy
    pthread_mutex_unlock(&proxy_mutex);
}
