#include <libreborn/util/exec.h>
#include <libreborn/log.h>

#ifdef MCPI_BUILD_RUNTIME
#include <trampoline/host.h>
extern "C" std::remove_pointer_t<trampoline_t> trampoline;
#endif

#include "bootstrap.h"

// Main
void start_runtime(const int argc, const char *argv[]) {
    // Set Debug Tag
    reborn_debug_tag = DEBUG_TAG("Runtime");

    // Run
#ifdef MCPI_BUILD_RUNTIME
    log_command(argv, DEFAULT_LOG_COMMAND_VERB " (Using Runtime)");
    runtime_main(trampoline, argc, argv);
#else
    (void) argc;
    safe_exec(argv);
#endif
}