project(symbols)

add_library(symbols INTERFACE)
target_include_directories(symbols INTERFACE include)

# Disable C++11 String ABI
target_compile_definitions(symbols INTERFACE -D_GLIBCXX_USE_CXX11_ABI=0)
