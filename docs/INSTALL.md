# Installation

## AppImage
Download packages [here](https://gitea.thebrokenrail.com/minecraft-pi-reborn/minecraft-pi-reborn/releases).

### System Requirements

* Debian Buster/Ubuntu 18.04 Or Higher
* FUSE 2
  * Debian/Ubuntu: `sudo apt install libfuse2`
  * Arch: `sudo pacman -S fuse2`
* Client-Only Dependencies
  * Graphics Drivers
  * GTK+ 3
    * Debian/Ubuntu: `sudo apt install libgtk-3-0`
    * Arch: `sudo pacman -S gtk3`
  * OpenAL
    * Debian/Ubuntu: `sudo apt install libopenal1`
    * Arch: `sudo pacman -S openal`

### Running
Follow [these](https://docs.appimage.org/introduction/quickstart.html#how-to-run-an-appimage) instructions.

## Flatpak
<a href="https://flathub.org/apps/details/com.thebrokenrail.MCPIReborn"><img width="240" alt="Download On Flathub" src="https://flathub.org/assets/badges/flathub-badge-en.svg" /></a>

### Note
Game data is stored in `~/.var/app/com.thebrokenrail.MCPIReborn/.minecraft-pi` instead of `~/.minecraft-pi`.

## Arch User Repository (Arch Linux Only)
The [`minecraft-pi-reborn-git`](https://aur.archlinux.org/packages/minecraft-pi-reborn-git) is available in the AUR.
