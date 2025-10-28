# Installation
There are multiple supported ways to install MCPI-Reborn.

## System Requirements
MCPI-Reborn requires support for OpenGL ES v1.5 (unlike the original game, which used OpenGL ES v1.1).
It also only supports Linux-based systems.

In addition, while the original game could only be run on the Raspberry Pi, MCPI-Reborn is much more flexible.
It supports running on 32-bit ARM (known as `armhf`), 64-bit ARM (known as `arm64`), and 64-bit x86 (known as `amd64`).

## Methods

### AppImage
The first supported way to install MCPI-Reborn is with an [AppImage](https://appimage.org).
An AppImage is a portable application format that allows users to run software without installation.

To run MCPI-Reborn, all you need to do is [download the latest AppImage](https://gitea.thebrokenrail.com/minecraft-pi-reborn/minecraft-pi-reborn/releases/latest) for your architecture and run it.

More details can be found [here](https://docs.appimage.org/introduction/quickstart.html#how-to-run-an-appimage).

> [!TIP]
> To integrate it with your application launcher, open the game launcher, click `About`, and click `Create Desktop Entry`.

<details>
<summary><b>Additional System Requirements</b></summary>
The AppImage requires <a href="https://www.debian.org/releases/bookworm/">Debian Bookworm</a> or higher.
This is equivalent to <a href="https://releases.ubuntu.com/jammy/">Ubuntu 22.04</a>.

It also requires some additional packages. To install them, run:
```sh
sudo apt install -y libopenal1 libglib2.0-0
```
</details>

### Flatpak
The next method is the official [Flatpak](https://www.flatpak.org/).
This method has the additional benefit of built-in sandboxing.
Unfortunately, it does not support 32-bit ARM systems.

It can be installed through [Flathub](https://flathub.org/apps/details/com.thebrokenrail.MCPIReborn).

### Debian Packages
On Debian-based systems (like Ubuntu),
another supported method is Debian (`.deb`) packages.

To install, run:
```sh
sudo curl https://gitea.thebrokenrail.com/api/packages/minecraft-pi-reborn/debian/repository.key -o /etc/apt/keyrings/gitea-minecraft-pi-reborn.asc
echo 'deb [signed-by=/etc/apt/keyrings/gitea-minecraft-pi-reborn.asc] https://gitea.thebrokenrail.com/api/packages/minecraft-pi-reborn/debian stable main' | sudo tee -a /etc/apt/sources.list.d/gitea.list
sudo apt update
sudo apt install minecraft-pi-reborn
```

### Pi-Apps
The final supported method is [Pi-Apps](https://github.com/Botspot/pi-apps).
It is a ["well-maintained collection of app installation-scripts"](https://github.com/Botspot/pi-apps#:~:text=well-maintained%20collection%20of%20app%20installation-scripts) that includes support for MCPI-Reborn.

The list of systems supported by Pi-Apps can be found [here](https://github.com/Botspot/pi-apps?tab=readme-ov-file#supported-systems).
