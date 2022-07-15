# Installation

## AppImage
Download packages [here](https://jenkins.thebrokenrail.com/job/minecraft-pi-reborn/job/master/lastSuccessfulBuild/artifact/out/).

### System Requirements
* Debian Buster/Ubuntu 18.04 Or Higher
* QEMU User-Mode
  * Debian/Ubuntu: ``sudo apt install qemu-user``
  * Arch: ``sudo pacman -Sy qemu-user``
* Client-Only Dependencies
  * Graphics Drivers
  * GTK+ 3
    * Debian/Ubuntu: ``sudo apt install libgtk-3-0``
    * Arch: ``sudo pacman -Sy gtk3``
  * OpenAL
    * Debian/Ubuntu: ``sudo apt install libopenal1``
    * Arch: ``sudo pacman -Sy openal``

### Running
Follow [these](https://docs.appimage.org/introduction/quickstart.html#how-to-run-an-appimage) instructions.

## Flatpak
<a href="https://flathub.org/apps/details/com.thebrokenrail.MCPIReborn"><img width="240" alt="Download On Flathub" src="https://flathub.org/assets/badges/flathub-badge-en.svg" /></a>

### Note
Game data is stored in ``~/.var/app/com.thebrokenrail.MCPIReborn/.minecraft-pi`` instead of ``~/.minecraft-pi``.
