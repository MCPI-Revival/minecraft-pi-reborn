#include <vector>
#include <cerrno>
#include <unistd.h>
#include <cstring>
#include <sys/prctl.h>
#include <csignal>
#include <exception>

#include "../common/common.h"

// Store Handlers
#define MAX_HANDLERS 100
static proxy_handler_t handlers[MAX_HANDLERS];
void _add_handler(unsigned char unique_id, proxy_handler_t handler) {
    if (unique_id >= MAX_HANDLERS) {
        PROXY_ERR("ID Too Big: %i", (int) unique_id);
    }
    if (handlers[unique_id] != NULL) {
        PROXY_ERR("Duplicate ID: %i", (int) unique_id);
    }
    handlers[unique_id] = handler;
}

// Store Parent PID
static int parent_is_alive = 1;
static void sigusr1_handler(__attribute__((unused)) int sig) {
    // Mark Parent As Dead
    parent_is_alive = 0;
}
// Check State Of Proxy And Exit If Invalid
void _check_proxy_state() {
    // Check Server State
    if (!parent_is_alive) {
        void_write_cache(); // Parent Is Dead, No Reason To Send A Dead Process Data
        PROXY_ERR("Server Terminated");
    }
}

// Exit Handler
static volatile int exit_requested = 0;
static void exit_handler(__attribute__((unused)) int signal_id) {
    // Request Exit
    exit_requested = 1;
}

// Main
int main(int argc, char *argv[]) {
    // Set Debug Tag
    reborn_debug_tag = PROXY_LOG_TAG;

    // Install Signal Handlers
    signal(SIGINT, SIG_IGN);
    struct sigaction act_sigterm;
    memset((void *) &act_sigterm, 0, sizeof (struct sigaction));
    act_sigterm.sa_handler = &exit_handler;
    sigaction(SIGTERM, &act_sigterm, NULL);

    // Send Signal On Parent Death To Interrupt Connection Read/Write And Exit
    prctl(PR_SET_PDEATHSIG, SIGUSR1);
    struct sigaction sa;
    memset((void *) &sa, 0, sizeof (struct sigaction));
    sa.sa_flags = SA_NOCLDSTOP;
    sa.sa_handler = &sigusr1_handler;
    if (sigaction(SIGUSR1, &sa, NULL) == -1) {
        PROXY_ERR("Unable To Install Signal Handler: %s", strerror(errno));
    }

    // Get Connection
    if (argc != 3) {
        PROXY_ERR("Invalid Arguments");
    }
    char *read_str = argv[1];
    char *write_str = argv[2];
    set_connection(atoi(read_str), atoi(write_str));
    PROXY_INFO("Connected");

    // Send Connection Message
    write_string((char *) CONNECTED_MSG);
    flush_write_cache();

    // Loop
    int running = is_connection_open();
    while (running && !exit_requested) {
        unsigned char unique_id = read_byte();
        if (handlers[unique_id] != NULL) {
            // Run Method
            handlers[unique_id]();
            // Check If Connection Is Still Open
            if (!is_connection_open()) {
                // Exit
                running = 0;
            } else {
                // Flush Write Cache
                flush_write_cache();
            }
        } else {
            PROXY_ERR("Invalid Method ID: %i", (int) unique_id);
        }
    }
    if (is_connection_open()) {
        close_connection();
    }

    // Exit
    PROXY_INFO("Stopped");
    return 0;
}
