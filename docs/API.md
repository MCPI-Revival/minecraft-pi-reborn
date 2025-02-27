---
gitea: none
include_toc: true
---

# API
In addition to Minecraft: Pi Edition's [builtin API](https://minecraft.wiki/w/Pi_Edition_protocol), MCPI-Reborn also implements the [RaspberryJuice API](https://github.com/zhuowei/RaspberryJuice). This page documents the API.

The API operates over a TCP socket.

It is typically hosted on port `4711`, but this can be configured.

## Compatibility Mode
By default, MCPI-Reborn runs in a "compatibility mode." This makes it completely compatible with RaspberryJuice, but limits functionality.

* **Bold** text only applies to the compatibility mode.
* <u>Underlined</u> text only applies when it is disabled.
* Text enclosed in curly braces (`{}`) is meant to be [Base64-URL](https://base64.guru/standards/base64url) encoded when the compatibility mode is disabled.
* In compatibility mode, entity type IDs are automatically translated to/from their MC Java equivalents.

## Commands
* Commands are formatted like `<command>(<args>)` and may return a response. The response `Fail` indicates an error.
* All commands are responses end with a newline.
* Arguments surrounded by square brackets (`[]`) are optional.
* Lists are delimited by `|`. For instance: `A|B|C`.
* Unless otherwise noted, all `player.*(...)` commands are equivalent to `entity.*(local_player_id,...)`.

### Vanilla
* `world.setBlock(x,y,z,block_id[,data])` 
  * Description: Set the specified block at the given location.
* `world.getBlock(x,y,z)`
  * Description: Retrieve the block ID at the specified location.
  * Output: `block_id`
* `world.getBlockWithData(x,y,z)`
  * Description: Retrieve the block ID and data value at the specified location.
  * Output: `block_id,data`
* `world.setBlocks(x0,y0,z0,x1,y1,x1,block_id[,data])`
  * Description: Fill the given region with the specified block.
* `world.getHeight(x,z)`
  * Description: Get the last (from the top-down) non-solid block's Y-coordinate at the given location.
  * Output: `max_y`
* `entity.setTile(entity_id,x,y,z)`
  * Description: Move the specified entity to the given integer position.
* `entity.getTile(entity_id)`
  * Description: Retrieve the given entity's position as integers.
  * Output: `x,y,z`
* `entity.setPos(entity_id,x,y,z)`
  * Description: Move the specified entity to the given floating-point position.
* `entity.getPos(entity_id)`
  * Description: Retrieve the given entity's position as floating-points.
  * Output: `x,y,z`
* `chat.post(message)`
  * Description: Post the specified message to chat.
* `camera.mode.setFixed()`
  * Description: Set the camera to fixed-position mode.
* `camera.mode.setNormal([entity_id])`
  * Description: Set the camera to normal mode. The camera will be the specified entity (or the local player if none is provided).
* `camera.mode.setFollow([entity_id])`
  * Description: Set the camera to follow mode. The camera will follow the specified entity (or the local player if none is provided).
* `camera.setPos(x,y,z)`
  * Description: Move the camera to the given floating-point position.
* `world.getPlayerIds()`
  * Description: Retrieve the entity IDs of all players.
  * Output: List of `entity_id`
* `world.checkpoint.save()`
  * Description: Save checkpoint.
* `world.checkpoint.restore()`
  * Description: Restore saved checkpoint.
* `player`/`world.setting(option,value)`
  * Description: Set the specified setting to the provided integer value (`0` is false, all other numbers are true). The possible settings are:
    * `autojump`
    * `nametags_visible`
    * `world_immutable`
* `events.clear()`
  * Description: Clear all queued events.
  * Note: On RaspberryJuice, this *does not* clear projectile events. This behavior is maintained only in compatibility mode.
* `events.block.hits()`
  * Description: Retrieve all queued block hit events.
  * Output: List of `block_x,block_y,block_z,block_face,entity_id`

### RaspberryJuice

### Reborn-Specific
* `reborn.disableCompatMode()`
  * Description: Disable the compatibility mode.
  * Output: `reborn_version`
* `reborn.enableCompatMode()`
  * Description: Re-enable the compatibility mode.
* `world.getSign(x,y,z)`
  * Description: Retrieve the text of the given sign.
  * Output: List of `{line}`
* `entity.getType(id)`
  * Description: Check the type of the given entity.
  * Output: `entity_id`