# Install SDK
install(EXPORT sdk DESTINATION "${MCPI_SDK_DIR}" FILE "sdk-targets.cmake")

# Write SDK Script
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/sdk.cmake"
    "${FORCE_SET_FUNCTION}"
    # Sanity Check
    "${ARM_SANITY_CHECK}"
    # Build Setup
    "${BUILD_MODE_SETUP}"
    "${COMPILE_FLAGS_SETUP}"
    # Log
    "message(STATUS \"Using Reborn SDK v${MCPI_VERSION}\")\n"
    # Include Targets
    "include(\"\${CMAKE_CURRENT_LIST_DIR}/sdk-targets.cmake\")\n"
)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/sdk.cmake" DESTINATION "${MCPI_SDK_DIR}")

# Calculate Hash Of SDK
string(CONCAT SDK_HASH_SCRIPT
    # Prepare
    "set(dir \"\$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}/${MCPI_SDK_DIR}\")\n"
    "set(out \"\${dir}/.hash\")\n"
    # Calculate Hashes
    "set(content \"\")\n"
    "file(GLOB_RECURSE files LIST_DIRECTORIES FALSE \"\${dir}/*\")\n"
    "foreach(file IN LISTS files)\n"
    "    file(SHA256 \"\${file}\" hash)\n"
    "    cmake_path(RELATIVE_PATH file BASE_DIRECTORY \"\${dir}\")\n"
    "    string(APPEND content \"\${hash} \${file}\\n\")\n"
    "endforeach()\n"
    # Write File
    "file(WRITE \"\${out}\" \"\${content}\")\n"
)
install(CODE "${SDK_HASH_SCRIPT}")