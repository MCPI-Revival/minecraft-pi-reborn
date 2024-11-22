---
gitea: none
include_toc: true
---

# Getting Started
Welcome to the official guide for setting up Minecraft: Pi Edition: Reborn (also known as MCPI-Reborn)! This document will help you through the installation and setup process.

## System Requirements
MCPI-Reborn requires support for OpenGL ES v2.0 (unlike the original game, which used OpenGL ES v1.1). It also only supports Linux-based systems.

In addition, while the original game could only be run on the Raspberry Pi, MCPI-Reborn is much more flexible. It supports running on 32-bit ARM (known as `armhf`), 64-bit ARM (known as `arm64`), and 64-bit x86 (known as `amd64`).

## Installation
There are three supported ways to install MCPI-Reborn.

### AppImage
The first supported way to install MCPI-Reborn is with an [AppImage](https://appimage.org). An AppImage is a portable application format that allows users to run software without installation.

To run MCPI-Reborn, all you need to do is [download the latest AppImage](https://gitea.thebrokenrail.com/minecraft-pi-reborn/minecraft-pi-reborn/releases/latest) for your architecture and run it.

More details can be found [here](https://docs.appimage.org/introduction/quickstart.html#how-to-run-an-appimage).

<details>
<summary><b>Additional System Requirements</b></summary>
The AppImage requires Debian Bullseye or higher. This is equivalent to Ubuntu 20.04.

It also requires some additional packages. To install them, run:
```sh
sudo apt install -y libfuse2 libopenal1 libglib2.0-0
```
</details>

### Flatpak
The next method is the official [Flatpak](https://www.flatpak.org/). This method has the additional benefit of built-in sandboxing. Unfortunately, it does not support 32-bit ARM systems.

It can be installed through [Flathub](https://flathub.org/apps/details/com.thebrokenrail.MCPIReborn).

### Pi-Apps
The final supported method is [Pi-Apps](https://github.com/Botspot/pi-apps). It is a ["well-maintained collection of app installation-scripts"](https://github.com/Botspot/pi-apps#:~:text=well-maintained%20collection%20of%20app%20installation-scripts) that includes support for MCPI-Reborn.

The list of systems supported by Pi-Apps can be found [here](https://github.com/Botspot/pi-apps?tab=readme-ov-file#supported-systems).

## Managing Game Data
Just as regular Minecraft stores game data at `~/.minecraft`, MCPI-Reborn uses `~/.minecraft-pi`[^1]. This is the profile directory and is where your worlds, screenshots, and game settings are stored.

The profile directory can easily be accessed by opening MCPI-Reborn and going to `Options -> Reborn -> Profile Directory`.

## Sound
One of MCPI-Reborn's most important modifications is the addition of a sound engine. However, due to copyright limitations, Minecraft's sounds cannot be distributed with MCPI-Reborn and must be installed manually.

Fortunately, installing the sound data is simple:
1. Obtain a valid Minecraft: Pocket Edition v0.6.1[^2] APK file.
2. Extract `lib/*/libminecraftpe.so` from the APK.
3. Create the directory `<Profile Directory>/overrides` if it does not already exist.
4. Copy `libminecraftpe.so` into `<Profile Directory>/overrides`.
5. Sound should now be fully functional!

## Custom Textures
MCPI-Reborn allows users to easily use custom textures through the use of an "overrides directory." Any files placed in this directory will automatically replace their equivalent file in MCPI-Reborn.

For instance, to override `data/images/terrain.png`, you would copy the replacement file to `<Overrides Folder>/images/terrain.png`.

The overrides directory is located at `<Profile Directory>/overrides`.

## Discord
If you have any questions or just want to talk about Minecraft: Pi Edition, there is an [official Discord server](https://discord.com/invite/aDqejQGMMy)!

[^1]: When using the Flatpak, the profile directory is located at `~/.var/app/com.thebrokenrail.MCPIReborn/.minecraft-pi`.
[^2]: This is not a strict requirement; a Minecraft: Pocket Edition v0.8.1 APK would likely work, but it is not guaranteed.