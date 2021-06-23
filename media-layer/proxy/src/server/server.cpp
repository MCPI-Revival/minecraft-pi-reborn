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
        safe_execvpe_relative_to_binary("lib/media-layer-proxy-client", argv, environ);
    } else {
        // Parent Process
        _client_pid = ret;
    }
    update_client_state(1, 0);
}

// Maximize Pipe Buffer Size
static void maximize_pipe_fd_size(int fd) {
    // Read Maximum Pipe Size
    std::ifstream max_size_file("/proc/sys/fs/pipe-max-size");
    if (!max_size_file.good()) {
        PROXY_ERR("%s", "Unable To Open Maximum Pipe Size File");
    }
    // Read One Line
    int max_size;
    std::string line;
    if (std::getline(max_size_file, line) && line.size() > 0) {
        max_size = std::stoi(line);
    } else {
        PROXY_ERR("%s", "Unable To Read Maximum Pipe Size File");
    }
    // Set Maximum Pipe Size
    errno = 0;
    if (fcntl(fd, F_SETPIPE_SZ, max_size) < max_size) {
        PROXY_ERR("Unable To Set Maximum Pipe Size: %s", errno != 0 ? strerror(errno) : "Unknown Error");
    }
}
static void maximize_pipe_size(int pipe[2]) {
    maximize_pipe_fd_size(pipe[0]);
    maximize_pipe_fd_size(pipe[1]);
}

// Start Server
__attribute__((constructor)) static void init_media_layer_proxy_server() {
    PROXY_INFO("%s", "Starting...");

    // Create Connection
    int server_to_client_pipe[2];
    safe_pipe2(server_to_client_pipe, 0);
    maximize_pipe_size(server_to_client_pipe);
    int client_to_server_pipe[2];
    safe_pipe2(client_to_server_pipe, 0);
    maximize_pipe_size(client_to_server_pipe);
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

// Assign Unique ID To Function
static std::unordered_map<std::string, unsigned char> &get_unique_ids() {
    static std::unordered_map<std::string, unsigned char> unique_ids;
    return unique_ids;
}
void _assign_unique_id(const char *name, unsigned char id) {
    get_unique_ids()[name] = id;
}
// Only Compare Strings The First Time, Then Cache C String Addresses
static std::unordered_map<const char *, unsigned char> &get_unique_ids_cache() {
    static std::unordered_map<const char *, unsigned char> unique_ids;
    return unique_ids;
}
static unsigned char get_unique_id(const char *name) {
    // Check If C String Is Cached
    if (get_unique_ids_cache().find(name) != get_unique_ids_cache().end()) {
        // Use Cache
        return get_unique_ids_cache()[name];
    } else {
        // Compare Strings
        unsigned char id = get_unique_ids()[name]; // Assume ID Exists
        get_unique_ids_cache()[name] = id;
        return id;
    }
}

// The Proxy Is Single-Threaded
static pthread_mutex_t proxy_mutex = PTHREAD_MUTEX_INITIALIZER;
void _start_proxy_call(const char *call_name) {
    // Lock Proxy
    pthread_mutex_lock(&proxy_mutex);
    write_byte(get_unique_id(call_name));
}
void end_proxy_call() {
    // Flush Write Cache
    flush_write_cache();
    // Release Proxy
    pthread_mutex_unlock(&proxy_mutex);
}
