# Manual Installation

## System Requirements
- At Least Debian/Raspbian Buster Or Ubuntu 20.04

## Before You Install

<details>
<summary>Debian/Raspbian Buster</summary>

### ``libseccomp2``
``minecraft-pi-reborn`` requires a newer version of the package ``libseccomp2`` to be installed when using Debian/Raspbian Buster.

```sh
# Install Backports Key
sudo apt-key adv --keyserver hkp://keyserver.ubuntu.com:80 --recv-keys 04EE7237B7D453EC
sudo apt-key adv --keyserver hkp://keyserver.ubuntu.com:80 --recv-keys 648ACFD622F3D138
# Install Backports Repository
echo 'deb http://deb.debian.org/debian buster-backports main' | sudo tee -a /etc/apt/sources.list
# Update APT Index
sudo apt update
# Install Updated libseccomp2
sudo apt install -t buster-backports libseccomp2
```

### Official Docker Package
``minecraft-pi-reborn`` requires the official Docker package when running Debian/Raspbian Buster instead of the Debian package (``docker.io``).

```sh
# Remove Debian Docker Package
sudo apt-get remove -y docker.io
# Install Official Docker Package
curl -fsSL https://get.docker.com -o get-docker.sh
sudo sh get-docker.sh
```

### Existing Installation
If you have un-modded ``minecraft-pi`` installed, you must remove it and transfer your existing worlds to ``minecraft-pi-reborn``'s folder.

```sh
# Transfer Worlds
mv ~/.minecraft ~/.minecraft-pi
# Remove Vanilla Minecraft Pi
sudo apt-get remove -y minecraft-pi
```

</details>

<details>
<summary>NVIDIA Users</summary>

The proprietary NVIDIA drivers are not supported, use either the open-source ``noveau`` drivers or use a different GPU (ie. Intel Integrated GPU).

</details>

## Installing
1. Download Appropriate Package From [Here](https://jenkins.thebrokenrail.com/job/minecraft-pi-reborn/job/master/lastSuccessfulBuild/artifact/out/deb/) (See Table Below To Pick Correct Package)
2. Install With ``sudo apt install ./<Path To File>`` Or Your Preferred Package Installer
3. Have Fun!

### Package Table
| Package | Description |
| --- | --- |
| ``minecraft-pi-reborn-virgl`` | Minecraft Pi Edition Using VirGL For Hardware Acceleration (Recommended For Desktop/Laptop) |
| ``minecraft-pi-reborn-native`` | Minecraft: Pi Edition Using Docker Device Mounting For GPU Acceleration (Recommended For ARM Devices (ie. Raspberry Pi, PinePhone, etc)) |
| ``minecraft-pi-reborn-server`` | Minecraft Pi Edition Modded Into A Dedicated Server |