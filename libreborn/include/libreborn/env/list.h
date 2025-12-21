// Configure Client
ENV(MCPI_FEATURE_FLAGS, "Client-Mode Feature Flags", "")
ENV(MCPI_USERNAME, "Player Username", "")
ENV(MCPI_RENDER_DISTANCE, "Render Distance", "")
ENV(MCPI_SERVER_LIST, "Server List", "")
// Game Assets
ENV(_MCPI_REBORN_ASSETS_PATH, "", "/p")
ENV(_MCPI_VANILLA_ASSETS_PATH, "", "/p")
// Command Line Arguments
ENV(_MCPI_BENCHMARK, "", "")
ENV(_MCPI_ONLY_GENERATE, "", "")
// Logging
ENV(_MCPI_LOG_FD, "", "")
ENV(MCPI_DEBUG, "Enable Debug Logging", "")
ENV(MCPI_QUIET, "Silence Most Logging", "")
// Server/Headless
ENV(_MCPI_SERVER_MODE, "", "")
ENV(_MCPI_FORCE_HEADLESS, "", "")
ENV(_MCPI_FORCE_NON_HEADLESS, "", "")
// Extra Configuration
ENV(MCPI_SKIN_SERVER, "Custom Skin Server", "")
ENV(MCPI_API_PORT, "Custom API Port", "")
ENV(MCPI_BLOCK_OUTLINE_WIDTH, "Custom Width For Block Outline (In Pixels)", "")
ENV(MCPI_GUI_SCALE, "Custom GUI Scale", "")
ENV(MCPI_BINARY, "Custom Game Binary", "/p")
ENV(MCPI_PROMOTIONAL, "Promotional Mdoe (Used For Consistent Screenshots)", "")
ENV(MCPI_DOWNLOAD_TIMEOUT, "Custom Timeout For Downloading Files (In Seconds)", "")
// $HOME
ENV(_MCPI_HOME, "", "/p")
ENV(MCPI_PROFILE_DIRECTORY, "Custom Profile Directory", "/p")
// Lock File
ENV(_MCPI_LOCK_FD, "", "")
// Taskbar Behavior (Used By Windows)
ENV(_MCPI_RELAUNCH_COMMAND, "", "")
ENV(_MCPI_RELAUNCH_DISPLAY_NAME_RESOURCE, "", "")