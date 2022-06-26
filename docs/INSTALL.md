# Installation

## AppImage
Download packages [here](https://jenkins.thebrokenrail.com/job/minecraft-pi-reborn/job/master/lastSuccessfulBuild/artifact/out/).

### Running
Follow [these](https://docs.appimage.org/introduction/quickstart.html#how-to-run-an-appimage) instructions.

### Picking A Package

#### Name Format
```
minecraft-pi-reborn-<Variant>_X.Y.Z_<Architecture>
```

#### Picking A Variant
* ``client``: Client mode, use this if you want to play MCPI.
* ``server``: Server mode, use this if you want to host a dedicated MCPI server.

#### Picking An Architecture
* ``amd64``: x86_64, use this if you are using a device with an AMD or Intel processor.
* ``armhf``: ARM 32-Bit, use this if you are using an ARM device (like a Raspberry Pi).
* ``arm64``: ARM 64-Bit, ``armhf`` but for 64-bit devices.

## Flatpak
<a href="https://flathub.org/apps/details/com.thebrokenrail.MCPIReborn"><img width="240" alt="Download On Flathub" src="https://flathub.org/assets/badges/flathub-badge-en.svg" /></a>

### Note
Game data is stored in ``~/.var/app/com.thebrokenrail.MCPIReborn/.minecraft-pi`` instead of ``~/.minecraft-pi``.
