# Architecture

## Launch Sequence

### Common

1. The launcher forks itself.
   1. The child process continues the launch sequence.
   2. The original process monitors the child process for crashes.

### Client

1. The launcher is started by the user.
   1. The launcher starts several Zenity dialogs to configure MCPI-Reborn.
      1. If the corresponding environmental variable for a setting is specified, it will be used instead of the dialog.
      2. If a setting is cached, then the dialog's default value will be the cached value instead of the normal default.
      3. When configuration has been completed, the settings specified will be cached.
2. The launcher replaces itself with MCPI.
   1. MCPI-Reborn components are loaded using `LD_PRELOAD` and `LD_LIBRARY_PATH`.
   2. If the Media Layer Proxy is enabled, then the Media Layer Proxy Client is started as a sub-process.

### Server

1. The launcher is started by the user.
2. The launcher replaces itself with MCPI.

## Components

### Launcher
This component configures the various environmental variables required for MCPI-Reborn to work. When running in client-mode, this component will also launch several Zenity dialogs for interactive configuration.

The environmental variables configured by this component includes:

* `LD_PRELOAD`
* `LD_LIBRARY_PATH`
* `MCPI_FEATURE_FLAGS`
* `MCPI_RENDER_DISTANCE`
* `MCPI_USERNAME`

This is always compiled for the host system's architecture.

### Media Layer
The Media Layer handles MCPI's graphics calls and user input. It replaces MCPI's native usage of SDL 1.2 with GLFW.

#### Core
This sub-component re-implements a subset of SDL 1.2 calls with GLFW. It also provides a few utility functions that are used internally by MCPI-Reborn.

The utility functions include:

* Fullscreen
* Audio
* Etc

This is always compiled for the host system's architecture unless the Media Layer Proxy is disabled.

This was created because SDL 1.2 has numerous bugs and is in-general unsupported.

#### Proxy
This sub-component must be used if the host system's architecture isn't ARM. It uses UNIX pipes to cross architectural boundaries and allow MCPI to use the Media Layer Core (which is always compiled for the host system's architecture).

It is made of two parts:

* Media Layer Proxy Server
  * Links To MCPI
  * Creates The UNIX Pipes
  * Same External API As The Media Layer Core
  * Compiled For ARM
* Media Layer Proxy Client
  * Links To The Media Layer Core
  * Connects To The Media Layer Proxy Server
  * Uses The System's Native GLES Driver (ie. Mesa)
  * Compiled For The Host System's Architecture

While proxying all Media Layer Core API calls across UNIX pipes does hurt performance, it is better than emulating the entire graphics stack.

Using this in server-mode is redundant (but is possible).

#### Extras
This sub-component contains code that must always be linked directly to MCPI.

This is always compiled for ARM.

#### Headers
This sub-component includes headers for SDL, GLES, and EGL allowing easy (cross-)compilation.

### Mods
This component patches MCPI to modify its behavior. It's loaded using `LD_PRELOAD`.

This is always compiled for ARM.

### `libreborn`
This component contains various utility functions including:

* Code Patching (ARM Only)
* Logging
* Etc

The code patching is ARM only because it relies on hard-coded ARM instructions. However, this is irrelevant since code patching is only needed in ARM code (to patch MCPI).

### `symbols`
This component contains all MCPI symbols.

## Dependencies
MCPI-Reborn has several dependencies:

* MCPI (Bundled)
* GLFW (Only In Client Mode; Bundled)
  * OpenGL ES 2.0
* OpenAL (Only In Client Mode)
* ZLib (Required By LibPNG; Bundled)
* LibPNG (Bundled)
* QEMU User Mode (Only On Non-ARM Hosts; Runtime Only)
* Zenity (Only In Client Mode; Runtime Only; Bundled)
