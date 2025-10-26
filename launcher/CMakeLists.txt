project(launcher)

# UI Code
add_library(launcher-ui SHARED
    src/ui/frame.cpp
    src/ui/color.cpp
)
target_link_libraries(launcher-ui
    reborn-util
    imgui
)

# Utility Functions
add_library(launcher-util SHARED
    src/util/util.cpp
    src/util/sdk.cpp
    src/util/env.cpp
)
target_link_libraries(launcher-util
    reborn-util
)

# Launcher
add_executable(launcher
    src/main.cpp
    src/client/configuration.cpp
    src/client/cache.cpp
    src/client/ui/window.cpp
    src/client/ui/main.cpp
    src/client/ui/advanced.cpp
    src/client/ui/servers.cpp
    src/client/ui/about.cpp
    src/updater/updater.cpp
)
if(MCPI_WIN32)
    target_sources(launcher PRIVATE
        src/options/parser-win32.cpp
        src/install/install-win32.cpp
    )
    target_link_libraries(launcher
        shlwapi
    )
else()
    target_sources(launcher PRIVATE
        src/options/parser.cpp
        src/install/install.cpp
    )
endif()
target_link_libraries(launcher
    launcher-ui
    launcher-util
    trampoline-headers
)

# Logger
add_executable(logger
    src/logger/logger.cpp
    src/logger/writer.cpp
)
target_link_libraries(logger
    reborn-util
    launcher-util
)

# Crash Report Dialog
add_executable(crash-report
    src/logger/crash-report.cpp
)
target_link_libraries(crash-report
    launcher-ui
)

# Bootstrapper
add_executable(bootstrap
    src/bootstrap/bootstrap.cpp
    src/bootstrap/download.cpp
    src/bootstrap/mods.cpp
    src/bootstrap/assets.cpp
    src/bootstrap/patchelf.cpp
    src/bootstrap/debug.cpp
    src/bootstrap/runtime.cpp
    src/bootstrap/util.cpp
)
if(MCPI_WIN32)
    target_sources(bootstrap PRIVATE
        src/bootstrap/wsl.cpp
    )
endif()
target_link_libraries(bootstrap
    reborn-util
    launcher-util
    LIB_LIEF
)
if(TARGET runtime)
    target_link_libraries(bootstrap
        media-layer-trampoline
        runtime
    )
endif()

# Metadata
add_subdirectory(data)

# Install
setup_library(launcher-ui TRUE FALSE)
setup_library(launcher-util TRUE FALSE)
function(install_bin target)
    target_link_libraries("${target}"
        launcher-meta
    )
    install(TARGETS "${target}" DESTINATION "${MCPI_INSTALL_DIR}")
    if(NOT MCPI_WIN32)
        # RPath
        set_target_properties("${target}" PROPERTIES
            INSTALL_RPATH "$ORIGIN/lib/native"
        )
        target_link_options("${target}" PRIVATE "LINKER:--disable-new-dtags")
    else()
        # Set Subsystem
        #set_target_properties("${target}" PROPERTIES
        #    WIN32_EXECUTABLE TRUE
        #)
        # Original Filename
        target_compile_definitions("${target}" PRIVATE
            # Windres Has Strange Quoting Rules
            "$<$<COMPILE_LANGUAGE:RC>:VER_FILENAME=\\\"$<TARGET_FILE_NAME:${target}>\\\">"
        )
    endif()
endfunction()
install_bin(launcher)
install_bin(logger)
install_bin(crash-report)
install_bin(bootstrap)
if(NOT MCPI_WIN32)
    install_symlink("../${MCPI_INSTALL_DIR}/launcher" "bin/${MCPI_APP_NAME}")
endif()

# AppImage
if(MCPI_IS_APPIMAGE_BUILD)
    install_symlink("bin/${MCPI_APP_NAME}" "AppRun")
    install_symlink("${MCPI_SHARE_DIR}/applications/${MCPI_APP_ID}.desktop" "${MCPI_APP_ID}.desktop")
    # Updater
    target_sources(launcher PRIVATE src/updater/appimage.cpp)
endif()