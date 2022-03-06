# Building

## Build Dependencies
* Common
  * ARM Compiler
  * Host Compiler (Clang)
  * CMake
* Host Architecture Dependencies
  * Client Mode Only
    * GLFW
    * FreeImage
    * OpenAL

## Runtime Dependencies
* Non-ARM Host Architectures
  * QEMU User-Mode
* Host Architecture Dependencies
  * CLient Mode Only
    * OpenGL ES 1.1
    * GLFW
    * FreeImage
    * OpenAL
    * Zenity

## Instructions
```sh
./scripts/build.sh <client|server> <armhf|arm64|i686|amd64>
```

### Custom CMake Arguments
```sh
./scripts/setup.sh <client|server> <armhf|arm64|i686|amd64> <Custom CMake Arguments>
./scripts/build.sh <client|server> <armhf|arm64|i686|amd64>
```
