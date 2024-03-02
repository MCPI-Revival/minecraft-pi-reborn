# Sound
One of MCPI-Reborn's main modifications is a sound-engine since MCPI does not include one by default[^1]. However, it can't be used out-of-box because MCPI does not contain any sound data and MCPI-Reborn can't include it because of copyright.

MCPE's sound data can be extracted from any MCPE v0.6.1[^2] APK file, just place its `libminecraftpe.so` into `~/.minecraft-pi/overrides`[^3] and you should have sound!

[^1]: The mute button is just leftover code from MCPE, it does not actually do anything in un-modded MCPI, however it is connected to MCPI-Reborn's sound-engine.
[^2]: This is not a hard limit, an MCPE v0.8.1 APK would probably work, but don't rely on it.
[^3]: On Flatpak, the path is `~/.var/app/com.thebrokenrail.MCPIReborn/.minecraft-pi/overrides`.
