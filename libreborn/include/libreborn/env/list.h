// Configure Client
ENV(MCPI_FEATURE_FLAGS, "Client-Mode Feature Flags")
ENV(MCPI_USERNAME, "Player Username")
ENV(MCPI_RENDER_DISTANCE, "Render Distance")
ENV(MCPI_SERVER_LIST, "Server List")
// Game Assets
ENV(_MCPI_REBORN_ASSETS_PATH, "")
ENV(_MCPI_VANILLA_ASSETS_PATH, "")
// Command Line Arguments
ENV(_MCPI_BENCHMARK, "")
ENV(_MCPI_ONLY_GENERATE, "")
// Logging
ENV(_MCPI_LOG_FD, "")
ENV(MCPI_DEBUG, "Enable Debug Logging")
// Server/Headless
ENV(_MCPI_SERVER_MODE, "")
ENV(_MCPI_FORCE_HEADLESS, "")
ENV(_MCPI_FORCE_NON_HEADLESS, "")
// Extra Configuration
ENV(MCPI_SKIN_SERVER, "Custom Skin Server")
ENV(MCPI_API_PORT, "Custom API Port")
ENV(MCPI_BLOCK_OUTLINE_WIDTH, "Custom Width For Block Outline (In Pixels)")
ENV(MCPI_GUI_SCALE, "Custom GUI Scale")
ENV(MCPI_BINARY, "Custom Game Binary")
// $HOME
ENV(_MCPI_HOME, "")
ENV(MCPI_PROFILE_DIRECTORY, "Custom Profile Directory")
// Lock File
ENV(_MCPI_LOCK_FD, "")