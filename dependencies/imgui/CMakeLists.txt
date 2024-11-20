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
    src/backends/imgui_impl_glfw.cpp
    src/backends/imgui_impl_opengl2.cpp
)
setup_header_dirs(imgui
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/backends"
)
find_package(OpenGL REQUIRED)
target_link_libraries(imgui PUBLIC glfw OpenGL::GL)

# Fonts
embed_resource(imgui src/misc/fonts/Roboto-Medium.ttf)
embed_resource(imgui src/misc/fonts/Cousine-Regular.ttf)

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
