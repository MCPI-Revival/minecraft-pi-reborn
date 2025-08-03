---
gitea: none
include_toc: true
---

# Getting Started
Welcome to the official guide for setting up Minecraft: Pi Edition: Reborn (also known as MCPI-Reborn)! This document will help you through the installation and setup process.

## Installation
Installation instructions and system requirements can be found [here](INSTALL.md).

## Managing Game Data
Just as regular Minecraft stores game data at `~/.minecraft`, MCPI-Reborn uses `~/.minecraft-pi`[^1]. This is the profile directory and is where your worlds, screenshots, and game settings are stored.

> [!TIP]
> The profile directory can easily be accessed by opening MCPI-Reborn and going to `Options -> Reborn -> Profile Directory`.

## Sound
One of MCPI-Reborn's most important modifications is the addition of a sound engine. However, due to copyright limitations, Minecraft's sounds cannot be distributed with MCPI-Reborn and must be installed manually.

Fortunately, installing the sound data is simple:
1. Obtain a valid Minecraft: Pocket Edition v0.6.1[^2] APK file.
2. Extract `lib/*/libminecraftpe.so` from the APK.
3. Create the directory `<Profile Directory>/overrides` if it does not already exist.
4. Copy `libminecraftpe.so` into the created directory.
5. Sound should now be fully functional!

## Custom Textures
MCPI-Reborn allows users to easily use custom textures through the use of an "overrides directory." Any files placed in this directory will automatically replace their equivalent file in MCPI-Reborn.

For instance, to override `data/images/terrain.png`, you would copy the replacement file to `<Overrides Folder>/images/terrain.png`.

The overrides directory is located at `<Profile Directory>/overrides`.

## Discord
If you have any questions or just want to talk about Minecraft: Pi Edition, there is an [official Discord server](https://discord.com/invite/aDqejQGMMy)!

[^1]: When using the Flatpak, the profile directory is located at `~/.var/app/com.thebrokenrail.MCPIReborn/.minecraft-pi`.
[^2]: This is not a strict requirement; a Minecraft: Pocket Edition v0.8.1 APK would likely work, but it is not guaranteed.
