# Manual Installation
[Download Packages Here](https://jenkins.thebrokenrail.com/job/minecraft-pi-reborn/job/master/lastSuccessfulBuild/artifact/out/)

## Picking A Package

### Name Format
```
minecraft-pi-reborn-<Variant>_X.Y.Z_<Architecture>
```

### Picking A Variant
* ``client``: Client mode, use this if you want to play MCPI
* ``server``: Server mode, use this if you want to host a dedicated MCPI server

### Picking An Architecture
* ``amd64``: x86_64, use this if you are using a device with an AMD or Intel processor
* ``armhf``: ARM 32-Bit, use this if you are using an ARM device (like a Raspberry Pi)
* ``arm64``: ARM 64-Bit, ``armhf`` but for 64-bit devices

### Download and Install the app

Download with your favourite browser. You can run it directly from your Downloads folder, but for long-term use, and for use by multiple users, it's preferable to move the file to /opt. Open the Terminal (often Accessories-> Terminal, or CTRL+T) and enter:
```
sudo mv Downloads/minecraft-pi-reborn-client-2.3.4-arm64.AppImage /opt/
```
(replacing Downloads/minecraft-pi-reborn-client-2.3.4-arm64.AppImage with the file you downloaded). 

To run, the file must be executable:
```
sudo chmod a+x /opt/minecraft-pi-reborn-client-2.3.4-arm64.AppImage 
```
### Run the app
Enter the name of the AppImage to run, e.g.:
```
/opt/minecraft-pi-reborn-client-2.3.4-arm64.AppImage
```
