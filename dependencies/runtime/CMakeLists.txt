project(runtime)

## Extra Runtime

# Build
add_subdirectory(src)

# RPath
if(TARGET runtime)
    set_target_properties(runtime PROPERTIES INSTALL_RPATH "$ORIGIN/../lib/native")
    target_link_options(runtime PRIVATE "LINKER:--disable-new-dtags")
endif()