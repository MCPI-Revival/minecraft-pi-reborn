cmake_minimum_required(VERSION 3.1.0)

project(launcher)

add_compile_options(-Wall -Wextra -Werror)

add_executable(launcher src/launcher.c)

# Install
install(TARGETS launcher DESTINATION /)
install(PROGRAMS src/run.sh DESTINATION /)