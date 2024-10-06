project(dependencies)

# stb_image
if(BUILD_ARM_COMPONENTS)
    add_subdirectory(stb_image)
endif()
# Minecraft: Pi Edition
if(BUILD_ARM_COMPONENTS AND NOT MCPI_OPEN_SOURCE_ONLY)
    add_subdirectory(minecraft-pi)
endif()
# Zenity (Minimal Build)
if(BUILD_NATIVE_COMPONENTS)
    add_subdirectory(zenity)
endif()
# LIEF
if(BUILD_NATIVE_COMPONENTS OR BUILD_MEDIA_LAYER_CORE)
    add_subdirectory(LIEF)
endif()
# Extra Runtime
add_subdirectory(runtime)
# GLFW
if(BUILD_MEDIA_LAYER_CORE)
    add_subdirectory(glfw)
endif()
# UTF8-CPP
add_subdirectory(utf8cpp)
# Symbol Prcoessor
if(BUILD_ARM_COMPONENTS)
    add_subdirectory(symbol-processor)
endif()
