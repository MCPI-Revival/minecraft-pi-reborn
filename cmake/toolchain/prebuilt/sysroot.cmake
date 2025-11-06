# Directories
set(sysroot_dir "${CMAKE_CURRENT_BINARY_DIR}/bundled-armhf-sysroot")
set(sysroot_dir_debug "${sysroot_dir}/debug")
set(sysroot_dir_release "${sysroot_dir}/release")

# Build Sysroot
function(_strip file)
    execute_process(
        COMMAND "${toolchain_dir}/bin/${target}-strip" "${file}"
        RESULT_VARIABLE ret
        ERROR_QUIET
    )
    # Delete Invalid Files
    if(NOT ret EQUAL 0)
        file(REMOVE "${file}")
    endif()
endfunction()
function(build_sysroot)
    # Create Directory
    file(REMOVE_RECURSE "${sysroot_dir}")

    # Copy Files From Toolchain
    file(MAKE_DIRECTORY "${sysroot_dir_debug}")
    file(COPY "${toolchain_dir}/${target}/sysroot/"
        DESTINATION "${sysroot_dir_debug}"
        USE_SOURCE_PERMISSIONS
        FILES_MATCHING
        PATTERN "*.so*"
    )

    # Delete Unneeded Files
    file(REMOVE_RECURSE "${sysroot_dir_debug}/usr/lib/audit")
    file(REMOVE_RECURSE "${sysroot_dir_debug}/usr/lib/gconv")

    # Strip Files
    file(MAKE_DIRECTORY "${sysroot_dir_release}")
    file(COPY "${sysroot_dir_debug}/." DESTINATION "${sysroot_dir_release}")
    file(GLOB_RECURSE files LIST_DIRECTORIES FALSE "${sysroot_dir_release}/*")
    foreach(file IN LISTS files)
        _strip("${file}")
    endforeach()
endfunction()

# Install Licenses
function(_install_license name)
    foreach(file IN LISTS ARGN)
        install(
            FILES "${toolchain_dir}/share/licenses/${name}/${file}"
            DESTINATION "${MCPI_LEGAL_DIR}/sysroot/${name}"
        )
    endforeach()
endfunction()
function(_install_licenses)
    _install_license(gcc
        COPYING.RUNTIME
    )
    _install_license(glibc
        COPYING
        COPYING.LIB
        LICENSES
    )
endfunction()

# Install Sysroot (Skipping Empty Directories)
function(_install_arm_sysroot_config config)
    # Get List Of Libraries
    set(dir "${sysroot_dir_${config}}")
    file(GLOB_RECURSE files
        LIST_DIRECTORIES FALSE
        RELATIVE "${dir}"
        "${dir}/*"
    )
    # Iterate
    foreach(file IN LISTS files)
        # Get File Path
        cmake_path(GET file PARENT_PATH parent)
        cmake_path(GET file FILENAME name)
        set(file "${dir}/${file}")
        if(MCPI_WIN32)
            # Windows Does Not Like Symlinks
            file(REAL_PATH "${file}" file)
        endif()
        # Install Library
        install(
            PROGRAMS "${file}"
            DESTINATION "${MCPI_INSTALL_DIR}/sysroot/${parent}"
            RENAME "${name}"
            CONFIGURATIONS "${config}"
        )
    endforeach()
endfunction()
function(install_arm_sysroot)
    _install_arm_sysroot_config(debug)
    _install_arm_sysroot_config(release)
    _install_licenses()
endfunction()