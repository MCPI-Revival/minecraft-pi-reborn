# Building

## Build Options
* ``MCPI_BUILD_MODE``
  * ``arm``: Only Build ARM Components
  * ``native``: Only Build Native Components
  * ``both`` (Default): Build Both ARM And Native Components For ARM
* ``MCPI_SERVER_MODE``
  * ``ON``: Enable Server Mode
  * ``OFF`` (Default): Disable Server Mode
* ``MCPI_HEADLESS_MODE``
  * ``ON`` (Default In Server Mode): Enable Headless Mode
  * ``OFF`` (Default In Client Mode): Disable Headless Mode
* ``MCPI_USE_MEDIA_LAYER_PROXY``
  * ``ON``: Enable The Media Layer Proxy
  * ``OFF`` (Default): Disable The Media Layer Proxy

## Build Dependencies
* Common
  * ARM Compiler
  * Host Compiler (Clang)
  * CMake
* Host Architecture Dependencies
  * Client Mode Only
    * GLFW
    * FreeImage

## Runtime Dependencies
* Non-ARM Host Architectures
  * QEMU User-Mode Static
* Host Architecture Dependencies
  * CLient Mode Only
    * OpenGL ES 1.1
    * GLFW
    * FreeImage
    * Zenity

## Two-Step Build
Use this when the host architecture is not ARM.

```sh
# Create Build Directory
mkdir build && cd build

# Build ARM Components
mkdir arm && cd arm
cmake -DMCPI_BUILD_MODE=arm ../..
make -j$(nproc) && sudo make install

# Build Native Components
mkdir native && cd native
cmake -DMCPI_BUILD_MODE=native ../..
make -j$(nproc) && sudo make install
```

## One-Step Build
Use this when the host architecture is ARM.

```sh
# Create Build Directory
mkdir build && cd build

# Build
cmake ..
make -j$(nproc) && sudo make install
```
