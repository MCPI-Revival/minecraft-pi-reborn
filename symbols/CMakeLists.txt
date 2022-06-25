project(symbols)

add_library(symbols INTERFACE)
target_include_directories(
    symbols
    INTERFACE
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
    "$<INSTALL_INTERFACE:${MCPI_SDK_INCLUDE_DIR}/symbols>"
)

# Disable C++11 String ABI
target_compile_definitions(symbols INTERFACE -D_GLIBCXX_USE_CXX11_ABI=0)

# SDK
install(TARGETS symbols EXPORT sdk DESTINATION "${MCPI_SDK_LIB_DIR}")
install(DIRECTORY "include/" DESTINATION "${MCPI_SDK_INCLUDE_DIR}/symbols")
