# Build Mode
set(MCPI_BUILD_MODE "native" CACHE STRING "\"arm\" = Build Only Code That Must Be ARM; \"native\" = Build Architecture-Independent Code")
set_property(CACHE MCPI_BUILD_MODE PROPERTY STRINGS "arm" "native")
if(MCPI_BUILD_MODE STREQUAL "arm")
    set(BUILD_ARM_COMPONENTS TRUE)
    set(BUILD_NATIVE_COMPONENTS FALSE)
elseif(MCPI_BUILD_MODE STREQUAL "native")
    set(BUILD_ARM_COMPONENTS FALSE)
    set(BUILD_NATIVE_COMPONENTS TRUE)
else()
    message(FATAL_ERROR "Invalid Mode")
endif()

# Specify Options
set(MCPI_OPTIONS "")
function(mcpi_option name description type default)
    set(full_name "MCPI_${name}")
    set("${full_name}" "${default}" CACHE "${type}" "${description}")
    list(APPEND MCPI_OPTIONS "-D${full_name}:${type}=${${full_name}}")
    set(MCPI_OPTIONS "${MCPI_OPTIONS}" PARENT_SCOPE)
endfunction()

# Prebuilt ARMHF Toolchain
if(BUILD_ARM_COMPONENTS)
    mcpi_option(USE_PREBUILT_ARMHF_TOOLCHAIN "Whether To Use A Prebuilt ARMHF Toolchain For Building ARM Components" BOOL FALSE)
    if(MCPI_USE_PREBUILT_ARMHF_TOOLCHAIN)
        include("${CMAKE_CURRENT_LIST_DIR}/prebuilt-armhf-toolchain.cmake")
    endif()
endif()
