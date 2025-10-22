project(launcher)

# 64-Bit FS Support
add_compile_definitions(_FILE_OFFSET_BITS=64)

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
if(NOT MCPI_WIN32)
    target_sources(launcher-util PRIVATE
        src/util/install.cpp
    )
endif()
target_link_libraries(launcher-util reborn-util)

# Launcher
add_executable(launcher WIN32
    src/bootstrap/bootstrap.cpp
    src/bootstrap/mods.cpp
    src/bootstrap/assets.cpp
    src/bootstrap/patchelf.cpp
    src/bootstrap/debug.cpp
    src/options/parser.cpp
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
target_link_libraries(launcher
    launcher-ui
    launcher-util
    LIB_LIEF
    trampoline-headers
    pthread
)

# Logger
add_executable(logger WIN32
    src/logger/logger.cpp
    src/logger/writer.cpp
)
target_link_libraries(logger
    reborn-util
    launcher-util
)

# Crash Report Dialog
add_executable(crash-report WIN32
    src/logger/crash-report.cpp
)
target_link_libraries(crash-report
    launcher-ui
)

# Runtime/Trampoline
add_executable(runtime-exe
    src/bootstrap/runtime.cpp
)
set_target_properties(runtime-exe PROPERTIES OUTPUT_NAME runtime)
target_link_libraries(runtime-exe reborn-util)
if(TARGET runtime)
    target_link_libraries(runtime-exe
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
    install(TARGETS "${target}" DESTINATION "${MCPI_INSTALL_DIR}")
    if(NOT MCPI_WIN32)
        # RPath
        set_target_properties("${target}" PROPERTIES INSTALL_RPATH "$ORIGIN/lib/native")
        target_link_options("${target}" PRIVATE "LINKER:--disable-new-dtags")
    else()
        # Manifest
        target_sources("${target}" PRIVATE "${MCPI_MANIFEST}")
    endif()
endfunction()
install_bin(launcher)
install_bin(logger)
install_bin(crash-report)
install_bin(runtime-exe)
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