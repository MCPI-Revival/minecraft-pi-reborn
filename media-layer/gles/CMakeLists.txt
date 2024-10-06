project(media-layer-gles)

# Build
add_library(GLESv1_CM OBJECT src/passthrough.cpp)
target_link_libraries(GLESv1_CM PRIVATE glfw media-layer-headers reborn-util)