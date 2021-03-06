# Manual Installation
[Download Packages Here](https://jenkins.thebrokenrail.com/job/minecraft-pi-reborn/job/master/lastSuccessfulBuild/artifact/out/)

## Picking A Package

### Name Format
```
minecraft-pi-reborn-<Variant>_X.Y.Z~<Distribution>_<Architecture>
```

### Picking A Variant
* ``client``: Client mode, use this if you want to play MCPI
* ``server``: Server mode, use this if you want to host a dedicated MCPI server

### Picking A Distribution
This specifies which version of Debian MCPI-Reborn was built against. Which one you should use depends on your current distribution. If your distribution supports it, you should use ``bullseye`` for better mouse sensitivity.

* Ubuntu 20.04+: ``bullseye``
* Raspberry Pi OS Buster: ``buster``
* Debian Bullseye+: ``bullseye``
* Debian Buster: ``buster``

### Picking An Architecture
* ``amd64``: x86_64, use this if you are using a device with an AMD or Intel processor
* ``armhf``: ARM 32-Bit, use this if you are using an ARM device (like a Raspberry Pi)
* ``arm64``: ARM 64-Bit, ``armhf`` but for 64-bit devices
