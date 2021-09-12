# Architecture

## Launch Sequence

### Client
1. The launcher is started by the user
   1. The launcher starts several Zenity dialogs to configure MCPI-Reborn
2. The launcher replaces itself with MCPI
   1. MCPI-Reborn components are loaded using ``LD_PRELOAD`` and ``LD_LIBRARY_PATH``
   2. If the Media Layer Proxy is enabled, the Media Layer Proxy Client is started as a sub-process

### Server
1. The launcher is started by the user
2. The launcher replaces itself with MCPI

## Components

### Launcher
This component configures the various environmental variables required for MCPI-Reborn to work. When running in client-mode, this component will also launch several Zenity dialogs for interactive configuration.

The environmental variables configured by this component includes:
* ``LD_PRELOAD``
* ``LD_LIBRAR_PATH``
* ``MCPI_FEATURE_FLAGS``
* ``MCPI_RENDER_DISTANCE``
* ``MCPI_USERNAME``

This is always compiled for the host system's architecture.

### Media Layer
The Media Layer handles MCPI's graphics calls and user input. It replaces MCPI's native usage of SDL 1.2 with GLFW.

#### Core
This sub-component re-implements a subset of SDL 1.2 calls with GLFW. It also provides a few utility functions that are used internally by MCPI-Reborn.

The utility functions include:
* Taking Screenshots
* Fullscreen
* Audio
* Etc

This is always compiled for the host system's architecture.

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

Using this in server-mode is redundant (and disallowed).

#### Extras
This sub-component contains code that must always be linked directly to MCPI.

This is always compiled for ARM.

#### Stubs
This sub-component implements stubs for various redundant libraries used by MCPI to silence linker errors.

This is always compiled for ARM.

##### What To Stub And What To Patch?
Most libraries (like ``bcm_host``) can just be replaced with stubs, because they don't need to do anything and aren't used by anything else. However, some libraries (like EGL and X11) might be used by some of MCPI-Reborn's dependencies (like GLFW) so instead of being replaced by a stub, each call is manually patched out from MCPI. A stub is still generated just in case that library isn't present on the system to silence linker errors, but it is only loaded if no other version is available.

#### Headers
This sub-component includes headers for SDL, GLES, and EGL allowing easy (cross-)compilation.

### Mods
This component links directly to MCPI and patches it to modify its behavior.

This is always compiled for ARM.

### ``libreborn``
This component contains various utility functions including:

* Code Patching (ARM Only)
* Logging
* Etc

The code patching is ARM only because it relies on hard-coded ARM instructions. However, this is irrelevant since code patching is only needed in ARM code (to patch MCPI).

### ``symbols``
This component contains all MCPI symbols.

## Dependencies
MCPI-Reborn has several dependencies:
* MCPI (Bundled)
* GLFW (Only In Client Mode)
  * Open GL ES 1.1
  * EGL
* OpenAL (Only In Client Mode)
* ZLib (Required By LibPNG; Bundled)
* LibPNG (Bundled)
* FreeImage (Only In Client Mode)
* QEMU User Mode (Only On Non-ARM Hosts; Runtime Only)
* Zenity (Only In Client Mode; Runtime Only)
