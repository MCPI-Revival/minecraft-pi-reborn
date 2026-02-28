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
function(mcpi_option name description type)
    set(full_name "MCPI_${name}")
    set("${full_name}" "${ARGN}" CACHE "${type}" "${description}")
    list(APPEND MCPI_OPTIONS "-D${full_name}:${type}=${${full_name}}")
    set(MCPI_OPTIONS "${MCPI_OPTIONS}" PARENT_SCOPE)
endfunction()

# Clear External CFLAGS When Building ARM Components
if(BUILD_ARM_COMPONENTS)
    unset(ENV{CFLAGS})
    unset(ENV{CXXFLAGS})
endif()