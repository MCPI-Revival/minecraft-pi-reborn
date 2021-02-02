# Minecraft: Pi Edition: Reborn
Minecraft: Pi Edition Modding Project

## Getting Started

### Debian/Raspbian Buster Users
1. Install Newer ``libseccomp2``
   ```sh
   # Install Backports Key
   sudo apt-key adv --keyserver keyserver.ubuntu.com --recv-keys 04EE7237B7D453EC
   sudo apt-key adv --keyserver keyserver.ubuntu.com --recv-keys 648ACFD622F3D138
   # Install Backports Repository
   echo 'deb http://deb.debian.org/debian buster-backports main' | sudo tee -a /etc/apt/sources.list
   # Update APT Index
   sudo apt update
   # Install Updated libseccomp2
   sudo apt install -t buster-backports libseccomp2
   ```
2. Install Official Docker Build
   ```sh
   curl -fsSL https://get.docker.com -o get-docker.sh
   sudo sh get-docker.sh
   ```
3. Remove Vanilla Minecraft: Pi Edition Package If Installed (``sudo apt remove minecraft-pi``)
4. Transfer Vanilla Minecraft: Pi Edition Worlds If Present (``mv ~/.minecraft ~/.minecraft-pi``)

### Installation Instructions
1. Download Appropriate Package From [Here](https://jenkins.thebrokenrail.com/job/minecraft-pi-reborn/job/master/lastSuccessfulBuild/artifact/out/deb/) (See Table Below)
2. Install With ``sudo apt install ./<Path To File>``
3. Have Fun!

#### Package Table
| Package | Description |
| --- | --- |
| ``minecraft-pi-reborn-server`` | Minecraft Pi Edition Modded Into A Dedicated Server |
| ``minecraft-pi-reborn-virgl`` | Minecraft Pi Edition Using VirGL For Hardware Acceleration (Recommended For Desktop) |
| ``minecraft-pi-reborn-native`` | Minecraft: Pi Edition Using Docker Device Mounting For GPU Acceleration (Recommended For ARM Devices (ie. Raspberry Pi, PinePhone, etc)) |

## Troubleshooting
Game logs are located in ``/tmp/minecraft-pi``.

### ``Couldn't connect to Docker daemon at http+docker://localhost - is it running?``
Start Docker if it isn't running:
```sh
sudo service docker start
```

### ``Error response from daemon: error gathering device information while adding custom device "/dev/dri": no such file or directory``
Make sure you are using the correct GPU drivers for your system.

If you are using a Raspberry Pi, make sure your GPU driver is set to ``Full KMS`` or ``Fake KMS`` in ``raspi-config``.

### ``Segmentation Fault`` (Exit Code: ``139``)
Report an issue with reproduction instructions and system details.

### ``[ERR]: Invalid ~/.minecraft-pi Permissions``
Update ``~/.minecraft-pi`` permissions:
```sh
sudo chown -R "$(id -u):$(id -g)" ~/.minecraft-pi
chmod -R u+rw ~/.minecraft-pi
```

## Dedicated Server
The dedicated server is a version of Minecraft: Pi Edition modified to run in a headless environment. It loads settings from a ``server.properties`` file.

To use, install the ``minecraft-pi-reborn-server`` package and run ``minecraft-pi-reborn-server``. It will generate the world and ``server.properties`` in the current directory.

This server is also compatible with MCPE Alpha v0.6.1.

### Limitations
- Player data is not saved because of limitations with MCPE LAN worlds
  - An easy workaround is to place your inventory in a chest before logging off
- Survival Mode servers are only compatible with ``minecraft-pi-reborn`` clients

## Modding
[View Modding](MODDING.md)

## Credits
[View Credits](CREDITS.md)
