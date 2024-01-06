project(symbol-processor)

# Install Dependencies
set(SRC "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(NODE_MODULES "${SRC}/node_modules")
function(npm_run)
    execute_process(
        COMMAND npm ${ARGV}
        WORKING_DIRECTORY "${SRC}"
        RESULT_VARIABLE RESULT
    )
    if(NOT RESULT EQUAL 0)
        file(REMOVE_RECURSE "${NODE_MODULES}")
        message(FATAL_ERROR "Unable To Run NPM Command")
    endif()
endfunction()
if(NOT EXISTS "${NODE_MODULES}")
    npm_run(ci --silent)
    npm_run(run --silent lint)
endif()
