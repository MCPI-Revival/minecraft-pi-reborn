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

### General Notes

#### Docker Versions
While the distribution-provided version of Docker works fine on most systems, in rare cases it can be outdated and cause bugs. Before reporting a bug, try using the official builds of Docker. These can be installed by following your distribution's instructions at https://docs.docker.com/engine/install.

### Distribution-Specific Notes

#### Raspbian Buster
By default Raspbian Buster ships an older version of the package ``libseccomp2``. This package is used to block certain dangerous system calls from running inside Docker containers. The included version accidentally blocks the system call ``clock_gettime64``, this causes bugs inside Minecraft: Pi Edition. However, the Debian ``buster-backports`` repo includes an updated version. You can enable the ``buster-backports`` repo and update ``libseccomp2`` by running:
```sh
echo 'deb http://deb.debian.org/debian buster-backports main' | sudo tee /etc/apt/sources.list
sudo apt update && sudo apt install libseccomp2
```

## Dedicated Server
The dedicated server is a version of Minecraft: Pi Edition modified to run in a headless environment. It loads settings from a ``server.properties`` file.

To use, install the ``minecraft-pi-server`` package and run ``minecraft-pi-server``. It will generate the world and ``server.properties`` in the current directory.

This is also compatible with MCPE 0.6.1.

### Limitations
- Player inventories are not saved because of limitations with MCPE LAN worlds
  - An easy workaround is to place your inventory in a chest before logging off
- Survival mode servers are only compatible with ``minecraft-pi-docker`` clients

## Modding
[View Modding](MODDING.md)

## Credits
[View Credits](CREDITS.md)
