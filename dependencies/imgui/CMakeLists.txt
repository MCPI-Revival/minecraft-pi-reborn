project(imgui)

# Silence Warnings
add_compile_options(-w)

## ImGui

# Build
add_library(imgui SHARED
    src/imgui.cpp
    src/imgui_draw.cpp
    src/imgui_tables.cpp
    src/imgui_widgets.cpp
    src/misc/cpp/imgui_stdlib.cpp
    src/backends/imgui_impl_glfw.cpp
    src/backends/imgui_impl_opengl2.cpp
)
setup_header_dirs(imgui
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/backends"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/misc/cpp"
)
find_package(OpenGL REQUIRED QUIET)
target_link_libraries(imgui PUBLIC glfw OpenGL::GL)

# Fonts
embed_resource(imgui src/misc/fonts/Roboto-Medium.ttf)
embed_resource(imgui src/misc/fonts/Cousine-Regular.ttf)

# Configure
target_compile_definitions(imgui PUBLIC
    IMGUI_DISABLE_DEMO_WINDOWS
    IMGUI_DISABLE_DEBUG_TOOLS
    IMGUI_DISABLE_DEFAULT_FONT
    IMGUI_DISABLE_OBSOLETE_FUNCTIONS
)

# Patch
execute_process(
    COMMAND "patch" "-p1" "--forward" "--reject-file=-"
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/src"
    INPUT_FILE "${CMAKE_CURRENT_SOURCE_DIR}/fix-hidpi.patch"
    OUTPUT_QUIET
)

# Install
setup_library(imgui TRUE TRUE)

# License
install(FILES src/LICENSE.txt src/docs/FONTS.md DESTINATION "${MCPI_LEGAL_DIR}/ImGui")
