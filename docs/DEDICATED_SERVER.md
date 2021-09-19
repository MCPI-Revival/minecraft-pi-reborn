# Dedicated Server
The dedicated server is a version of Minecraft: Pi Edition modified to run in a headless environment. It loads settings from a ``server.properties`` file.

This server is also compatible with MCPE Alpha v0.6.1.

## Setup

### Debian Package
To use, install and run ``minecraft-pi-reborn-server``. It will generate the world and ``server.properties`` in the current directory.

### Docker Image
An official Docker image is also provided: [thebrokenrail/minecraft-pi-reborn-server](https://hub.docker.com/r/thebrokenrail/minecraft-pi-reborn-server).

## Server Limitations
* Player data is not saved because of limitations with MCPE LAN worlds
  * An easy workaround is to place your inventory in a chest before logging off
* Survival Mode servers are incompatible with unmodded MCPI
