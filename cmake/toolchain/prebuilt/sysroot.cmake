# Directories
set(sysroot_dir "${CMAKE_CURRENT_BINARY_DIR}/bundled-armhf-sysroot")
set(sysroot_dir_debug "${sysroot_dir}/debug")
set(sysroot_dir_release "${sysroot_dir}/release")

# Strip File
function(_strip file)
    execute_process(
        COMMAND "${toolchain_dir}/${toolchain_name}/bin/${target}-strip" "${file}"
        RESULT_VARIABLE ret
        ERROR_QUIET
    )
    # Delete Invalid Files
    if(NOT ret EQUAL 0)
        file(REMOVE "${file}")
    endif()
endfunction()

# Build Sysroot
function(build_sysroot)
    # Remove Old Directory
    file(REMOVE_RECURSE "${sysroot_dir}")

    # Copy Files From Toolchain
    file(MAKE_DIRECTORY "${sysroot_dir_debug}")
    file(COPY "${toolchain_dir}/${toolchain_name}/${target}/sysroot/lib/."
        DESTINATION "${sysroot_dir_debug}"
        FILES_MATCHING
        PATTERN "*.so*"
    )

    # Strip Files
    file(MAKE_DIRECTORY "${sysroot_dir_release}")
    file(COPY "${sysroot_dir_debug}/." DESTINATION "${sysroot_dir_release}")
    file(GLOB files "${sysroot_dir_release}/*")
    foreach(file IN LISTS files)
        _strip("${file}")
    endforeach()
endfunction()

# Install Licenses
function(_install_license name)
    foreach(file IN LISTS ARGN)
        install(
            FILES "${toolchain_dir}/${toolchain_name}/share/licenses/${name}/${file}"
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

# Install Sysroot
function(_install_arm_sysroot_config config)
    # Get List Of Libraries
    set(dir "${sysroot_dir_${config}}")
    file(GLOB files "${dir}/*")

    # Install Libraries
    foreach(file IN LISTS files)
        # Get File Path
        cmake_path(GET file FILENAME name)
        if(MCPI_WIN32)
            # Windows Does Not Like Symlinks
            file(REAL_PATH "${file}" file)
        endif()

        # Install Library
        install(
            FILES "${file}"
            DESTINATION "${MCPI_INSTALL_DIR}/sysroot"
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