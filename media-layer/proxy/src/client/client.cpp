#include <vector>
#include <cerrno>
#include <unistd.h>
#include <cstring>
#include <sys/prctl.h>
#include <csignal>

#include "../common/common.h"

// Store Handlers
__attribute__((const)) static std::vector<proxy_handler_t> &get_handlers() {
    static std::vector<proxy_handler_t> handlers;
    return handlers;
}
void _add_handler(unsigned char unique_id, proxy_handler_t handler) {
    if (get_handlers().size() > unique_id && get_handlers()[unique_id] != NULL) {
        PROXY_ERR("Duplicate ID: %i", (int) unique_id);
    }
    if (get_handlers().size() <= unique_id) {
        get_handlers().resize(unique_id + 1);
    }
    get_handlers()[unique_id] = handler;
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

// Main
int main(int argc, char *argv[]) {
    // Ignore SIGINT, Send Signal To Parent
    signal(SIGINT, SIG_IGN);

    // Send Signal On Parent Death To Interrupt Connection Read/Write And Exit
    prctl(PR_SET_PDEATHSIG, SIGUSR1);
    struct sigaction sa;
    sigemptyset(&sa.sa_mask);
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
    while (running) {
        unsigned char unique_id = read_byte();
        if (get_handlers().size() > unique_id && get_handlers()[unique_id] != NULL) {
            // Run Method
            get_handlers()[unique_id]();
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

    // Exit
    PROXY_INFO("Stopped");
    return 0;
}
