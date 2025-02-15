project(symbol-processor)

# Install Dependencies
set(SRC "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(NODE_MODULES "${SRC}/node_modules")
execute_process(
    COMMAND npm ci --silent
    WORKING_DIRECTORY "${SRC}"
    COMMAND_ERROR_IS_FATAL ANY
)
