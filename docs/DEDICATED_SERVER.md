# Dedicated Server
The dedicated server is a version of Minecraft: Pi Edition modified to run in a headless environment. It loads settings from a ``server.properties`` file.

This server is also compatible with MCPE Alpha v0.6.1.

## Setup

### Debian Package
To use, install the ``minecraft-pi-reborn-server`` package and run ``minecraft-pi-reborn-server`` or use. It will generate the world and ``server.properties`` in the current directory.

### Docker Compose
Make sure you have ``qemu-user-static`` installed and ``binfmt`` setup.
```yml
version: "3.7"

services:
  minecraft-pi:
    image: thebrokenrail/minecraft-pi-reborn:server
    volumes:
      - ./minecraft-pi/data:/home/.minecraft-pi
      - /usr/bin/qemu-arm-static:/usr/bin/qemu-arm-static
    restart: always
    stdin_open: true
    tty: true
    ports:
      - "19132:19132/udp"
```

## Server Limitations
- Player data is not saved because of limitations with MCPE LAN worlds
  - An easy workaround is to place your inventory in a chest before logging off
- Survival Mode servers are only compatible with ``minecraft-pi-reborn`` clients