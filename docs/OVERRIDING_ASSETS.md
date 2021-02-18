# Overriding Assets
Normally, Minecraft: Pi Edition assets can be easily overridden by physically replacing the file, however ``minecraft-pi-=reborn`` uses a Docker image making this much harder to do. To make overriding assets easier, ``minecraft-pi-reborn`` provides an overrides folder. Any file located in Minecraft: Pi Edition's ``data`` folder can be overridden by placing a file with the same name and path in the overrides folder. The overrides folder is located at ``~/.minecraft-pi/overrides``.

## Examples
- ``data/images/terrain.png`` -> ``~/.minecraft-pi/overrides/images/terrain.png``
- ``data/lang/en_US.lang`` -> ``~/.minecraft-pi/overrides/lang/en_US.lang``