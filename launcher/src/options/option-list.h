OPTION(debug, "debug", 'd', "Enable Debug Logging (" MCPI_DEBUG_ENV ")")
OPTION(copy_sdk, "copy-sdk", -2, "Extract Modding SDK And Exit")
OPTION(disable_crash_report, "disable-crash-report", -1, "Disable Crash Report Dialog")
#ifndef MCPI_SERVER_MODE
OPTION(use_default, "default", -3, "Skip Configuration Dialogs")
OPTION(no_cache, "no-cache", -4, "Disable Configuration Cache")
OPTION(wipe_cache, "wipe-cache", -5, "Wipe Cached Configuration")
OPTION(print_available_feature_flags, "print-available-feature-flags", -6, "Print Available Feature Flags")
OPTION(benchmark, "benchmark", -7, "Run Benchmark")
#else
OPTION(only_generate, "only-generate", -8, "Generate World And Exit")
#endif
#ifdef MCPI_USE_NATIVE_TRAMPOLINE
OPTION(use_ptrace_trampoline, "use-ptrace-trampoline", -9, "Use PTrace For Calling Host Functions Instead Of Pipes")
#endif