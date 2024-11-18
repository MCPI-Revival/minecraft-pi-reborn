#include <libreborn/libreborn.h>

#include "logger.h"

// Show Crash Report Dialog
#define DIALOG_TITLE "Crash Report"
#define CRASH_REPORT_DIALOG_WIDTH "640"
#define CRASH_REPORT_DIALOG_HEIGHT "480"
void show_report(const char *log_filename) {
    // Fork
    pid_t pid = fork();
    if (pid == 0) {
        // Child
        setsid();
        ALLOC_CHECK(freopen("/dev/null", "w", stdout));
        ALLOC_CHECK(freopen("/dev/null", "w", stderr));
        ALLOC_CHECK(freopen("/dev/null", "r", stdin));
        const char *command[] = {
            "zenity",
            "--title", DIALOG_TITLE,
            "--name", MCPI_APP_ID,
            "--width", CRASH_REPORT_DIALOG_WIDTH,
            "--height", CRASH_REPORT_DIALOG_HEIGHT,
            "--text-info",
            "--text", MCPI_APP_TITLE " has crashed!\n\nNeed help? Consider asking on the <a href=\"" MCPI_DISCORD_INVITE "\">Discord server</a>! <i>If you believe this is a problem with " MCPI_APP_TITLE " itself, please upload this crash report to the #bugs Discord channel.</i>",
            "--filename", log_filename,
            "--no-wrap",
            "--font", "Monospace",
            "--save-filename", MCPI_VARIANT_NAME "-crash-report.log",
            "--ok-label", "Exit",
            nullptr
        };
        safe_execvpe(command, (const char *const *) environ);
    }
}