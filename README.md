# Minecraft: Pi Edition For Docker
This is a project allowing Minecraft: Pi Edition to be run without a Raspberry Pi using Docker.

## Setup
[View Binaries](https://jenkins.thebrokenrail.com/job/minecraft-pi-docker/job/master/lastSuccessfulBuild/artifact/out/)

### Packages
| Package | Description |
| --- | --- |
| ``minecraft-pi-server`` | Dedicated Server |
| ``minecraft-pi-virgl`` | Minecraft Pi Edition Using VirGL For Hardware Acceleration (Recommended For Desktop) |
| ``minecraft-pi-native`` | Minecraft: Pi Edition Using Docker Device Mounting For GPU Acceleration (Recommended For Raspberry Pi) |

## Dedicated Server
The dedicated server is a version of Minecraft: Pi Edition modified to run in a headless environment. It loads settings from a ``server.properties`` file.

To use, install the ``minecraft-pi-server`` package and run ``minecraft-pi-server``. It will generate the world and ``server.properties`` in the current directory.

This is also compatible with MCPE 0.6.1.

### Limitations
- Player data is not saved because of limitations with MCPE LAN worlds
  - An easy workaround is to place your inventory in a chest before logging off
- Survival mode servers are only compatible with ``minecraft-pi-docker`` clients

## Modding
[View Modding](MODDING.md)
