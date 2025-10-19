#include <libreborn/util/exec.h>

#ifdef MCPI_BUILD_RUNTIME
#include <trampoline/host.h>
extern "C" std::remove_pointer_t<trampoline_t> trampoline;
#endif

// Main
int main(int argc, const char *argv[]) {
    argc--;
    argv++;
#ifdef MCPI_BUILD_RUNTIME
    runtime_main(trampoline, argc, argv);
#else
    (void) argc;
    safe_execvpe(argv);
#endif
}