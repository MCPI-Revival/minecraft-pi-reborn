project(symbol-processor)

# Install Dependencies
force_set(SYMBOL_PROCESSOR_SRC "${CMAKE_CURRENT_SOURCE_DIR}/src" PATH)
find_program(SYMBOL_PROCESSOR_NPM
    NAMES npm
    REQUIRED
)
execute_process(
    COMMAND "${SYMBOL_PROCESSOR_NPM}" ci --silent
    WORKING_DIRECTORY "${SYMBOL_PROCESSOR_SRC}"
    COMMAND_ERROR_IS_FATAL ANY
)

# Build Library
function(add_symbol_library name)
    # Directories
    set_and_mkdir(GENERATED_DIR "${CMAKE_CURRENT_BINARY_DIR}/generated/${name}")
    set_and_mkdir(INCLUDE_OUTPUT_DIR "${GENERATED_DIR}/include")
    set_and_mkdir(HEADER_OUTPUT_DIR "${INCLUDE_OUTPUT_DIR}/symbols")
    set_and_mkdir(SRC_OUTPUT_DIR "${GENERATED_DIR}/src")

    # Process Input/Output Files
    set(OUTPUT_FILES_HEADERS "")
    set(OUTPUT_FILES_SOURCES "")
    set(INPUT_FILES "")
    foreach(file IN LISTS ARGN)
        list(APPEND INPUT_FILES "${CMAKE_CURRENT_SOURCE_DIR}/${file}")
        # Handle Structure Definition Files
        cmake_path(GET file EXTENSION file_ext)
        if(file_ext STREQUAL ".def")
            cmake_path(GET file STEM file)
            list(APPEND OUTPUT_FILES_SOURCES "${SRC_OUTPUT_DIR}/${file}.cpp")
            list(APPEND OUTPUT_FILES_HEADERS "${HEADER_OUTPUT_DIR}/${file}.h")
        endif()
    endforeach()
    list(APPEND OUTPUT_FILES_HEADER "${HEADER_OUTPUT_DIR}/__common.h")
    set(OUTPUT_FILES
        ${OUTPUT_FILES_HEADERS}
        ${OUTPUT_FILES_SOURCES}
    )

    # Generate
    list(LENGTH INPUT_FILES INPUT_FILES_LENGTH)
    add_custom_command(OUTPUT ${OUTPUT_FILES}
        DEPENDS ${INPUT_FILES}
        COMMAND
            "${SYMBOL_PROCESSOR_NPM}" "start" "--silent" "--"
            "${SRC_OUTPUT_DIR}" "${HEADER_OUTPUT_DIR}"
            ${INPUT_FILES}
        WORKING_DIRECTORY "${SYMBOL_PROCESSOR_SRC}"
        COMMENT "Processing ${INPUT_FILES_LENGTH} Symbol Definition Files..."
        VERBATIM
    )

    # Build
    add_library("${name}" SHARED
        ${INPUT_FILES}
        ${OUTPUT_FILES}
    )

    # Setup Headers
    setup_header_dirs(symbols "${INCLUDE_OUTPUT_DIR}")

    # Export Variables
    set("${name}_HEADER_DIR" "${INCLUDE_OUTPUT_DIR}" PARENT_SCOPE)
    set("${name}_HEADERS" "${OUTPUT_FILES_HEADERS}" PARENT_SCOPE)
endfunction()