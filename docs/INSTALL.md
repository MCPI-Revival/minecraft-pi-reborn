---
gitea: none
include_toc: true
---

# Installation
There are multiple supported ways to install MCPI-Reborn.

## System Requirements
The original game only supported running on the Raspberry Pi.
However, this project is much more flexible:

* Operating System:
  * GNU/Linux
  * Windows 10/11
* Hardware:
  * 32-Bit ARM (Known As `armhf`)
  * 64-Bit ARM (Known As `arm64`)
  * 64-Bit x86 (Known As `amd64`)
* Graphics:
  * OpenGL v1.5 Or Higher

## Instructions

> [!TIP]
> To integrate the game with your desktop environment,
> open the game launcher, select `About`, and click `Create Desktop Entry`.

### AppImage (Linux)
The first supported way to install MCPI-Reborn is with an [AppImage](https://appimage.org).
An AppImage is a portable application format that allows users to run software without installation.

To run MCPI-Reborn,
all you need to do is download the [latest AppImage](https://gitea.thebrokenrail.com/minecraft-pi-reborn/minecraft-pi-reborn/releases/latest) for your architecture and run it.

More details can be found [here](https://docs.appimage.org/introduction/quickstart.html#how-to-run-an-appimage).

<details>
<summary><b>Additional System Requirements</b></summary>
The AppImage requires <a href="https://www.debian.org/releases/bookworm/">Debian Bookworm</a> or higher.
This is equivalent to <a href="https://releases.ubuntu.com/jammy/">Ubuntu 22.04</a>.

It also requires some additional packages. To install them, run:
```sh
sudo apt install -y libopenal1 libglib2.0-0
```
</details>

### Flatpak (Linux)
The next method is the official [Flatpak](https://www.flatpak.org/).
This method has the additional benefit of built-in sandboxing.
Unfortunately, it does not support 32-bit ARM systems.

It can be installed through [Flathub](https://flathub.org/apps/details/com.thebrokenrail.MCPIReborn).

### Debian Packages (Linux)
On Debian-based systems (like Ubuntu),
another supported method is Debian packages (`.deb` files).

To install, run:
```sh
sudo curl https://gitea.thebrokenrail.com/api/packages/minecraft-pi-reborn/debian/repository.key -o /etc/apt/keyrings/minecraft-pi-reborn.asc
echo 'deb [signed-by=/etc/apt/keyrings/minecraft-pi-reborn.asc] https://gitea.thebrokenrail.com/api/packages/minecraft-pi-reborn/debian stable main' | sudo tee -a /etc/apt/sources.list.d/minecraft-pi-reborn.list
sudo apt update
sudo apt install minecraft-pi-reborn
```

### Pi-Apps (Linux)
On [certain supported system](https://github.com/Botspot/pi-apps?tab=readme-ov-file#supported-systems),
it can also be installed from [Pi-Apps](https://github.com/Botspot/pi-apps).
This is a ["well-maintained collection of app installation-scripts"](https://github.com/Botspot/pi-apps#:~:text=well-maintained%20collection%20of%20app%20installation-scripts) that includes support for MCPI-Reborn.

### Windows
MCPI-Reborn also supports Windows 10/11.

This requires at least Windows 10 version 1903 (build 18362).
It internally relies on [WSL 1](https://learn.microsoft.com/en-us/windows/wsl/compare-versions).

Installation is relatively simple:

1. Install WSL 1 (if it is not already installed):
   ```powershell
   wsl --install --enable-wsl1 --no-distribution
   ```
   This will require a reboot afterward.
2. Create a dedicated WSL container for MCPI-Reborn:
   ```powershell
   wsl --install --distribution Ubuntu-24.04 --version 1 --name MCPI-Reborn
   ```
   This will ask you to set a username and password for the container.
   These values do not matter and can be anything.
3. Download the [latest ZIP archive](https://gitea.thebrokenrail.com/minecraft-pi-reborn/minecraft-pi-reborn/releases/latest) for Windows.
4. Extract it.
5. Run `launcher.exe`.
