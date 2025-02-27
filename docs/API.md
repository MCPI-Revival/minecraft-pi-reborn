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
* <ins>Underlined</ins> text only applies when it is disabled.
* Text enclosed in curly braces (for instance `{text}`) is meant to be [Base64-URL](https://base64.guru/standards/base64url)-encoded when the compatibility mode is disabled.
* In compatibility mode, entity type IDs are automatically translated to/from their [MC Java equivalents](https://mcreator.net/wiki/entity-ids#toc-index-2).

## Commands
* Commands are formatted like `<command>(<args>)` and may return a response. The response `Fail` indicates an error.
* All commands are responses end with a newline.
* Arguments surrounded by square brackets (for instance `[abc]`) are optional.
* Numbers surrounded by colons (for instance `:a:`) are floating-point, all other numbers are integers.
* Lists are delimited by pipes (`|`). For instance: `A|B|C`.
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
* `entity.setTile(entity_id,x,y,z)`
  * Description: Move the specified entity to the given position.
* `entity.setPos(entity_id,:x:,:y:,:z:)`
  * Description: See above.
* `entity.getTile(entity_id)`
  * Description: Retrieve the given entity's position.
  * Output: `x,y,z`
* `entity.getPos(entity_id)`
  * Description: See above.
  * Output: `:x:,:y:,:z:`
* `chat.post(message)`
  * Description: Post the specified message to chat.
* `camera.mode.setFixed()`
  * Description: Set the camera to fixed-position mode.
* `camera.mode.setNormal([entity_id])`
  * Description: Set the camera to normal mode. The camera will be the specified entity (or the local player if none is provided).
* `camera.mode.setFollow([entity_id])`
  * Description: Set the camera to follow mode. The camera will follow the specified entity (or the local player if none is provided).
* `camera.setPos(:x:,:y:,:z:)`
  * Description: Move the camera to the given position. The XZ-coordinates are automatically offset by `0.5`.
* `events.clear()`
  * Description: Clear all queued events.
  * Note: On RaspberryJuice, this *does not* clear projectile events. This behavior is maintained only in compatibility mode.
* `events.block.hits()`
  * Description: Retrieve all queued block hit events.
  * Output: List of `x,y,z,face,entity_id`

### RaspberryJuice
* `world.getBlocks(x0,y0,z0,x1,y1,x1)`
  * Description: Retrieve the blocks in the specified region.
  * Output: List of <code>block_id<ins>,data</ins></code>
    * In compatibility mode, this list is delimited with commas (`,`).
* `world.getPlayerId({username})`
  * Description: Retrieve the entity ID of the specified player.
  * Output: `entity_id`
* `world.getEntities(entity_type_id)`
  * Description: Retrieve all entities of the specified type[^1].
  * Output: List <code>entity_id,entity_type_id<b>,entity_type_name</b>,:x:,:y:,:z:</code>
* `entity.getEntities(entity_id,distance,entity_type_id)`
  * Description: Retrieve all entities of the specified type[^1] within the given distance of the provided entity.
  * Output: See above.
* `world.removeEntity(entity_id)`
  * Description: Remove the specified entity.
  * Output: `number_of_entities_removed`
* `world.removeEntities(entity_type_id)`
  * Description: Remove all entities of the specified type[^1].
  * Output: See above.
* `entity.removeEntities(entity_id,distance,entity_type_id)`
  * Description: Remove all entities of the specified type[^1] within the given distance of the provided entity.
  * Output: See above.
* `world.spawnEntity(x,y,x,entity_type_id)`
  * Description: Spawn the specified entity at the given position.
  * Output: `entity_id`
* `world.getEntityTypes()`
  * Description: Retrieve all supported entity types.
  * Output: List of `entity_type_id,entity_type_name`
* `world.setSign(x,y,z,id,data[,{line_1}][,{line_2}][,{line_3}][,{line_4}])`
  * Description: Set the specified block at the given location. If the block is a sign, then also set its contents.
* `entity.getName(entity_id)`
  * Description: Retrieve the name of the specified entity.
  * Output: `{entity_name}`
* `entity.setDirection(entity_id,:x:,:y:,:z:)`
  * Description: Set the specified entity's rotation using a unit vector.
* `entity.getDirection(entity_id)`
  * Description: Retrieve the specified entity's rotation as a unit vector.
  * Output: `:x:,:y:,:z:`
* `entity.setRotation(entity_id,:yaw:)`
  * Description: Set the specified entity's yaw.
* `entity.getRotation(entity_id)`
  * Description: Retrieve the specified entity's yaw.
  * Output: `:yaw:`
* `entity.setPitch(entity_id,:pitch:)`
  * Description: Set the specified entity's pitch.
* `entity.getPitch(entity_id)`
  * Description: Retrieve the specified entity's pitch.
  * Output: `:pitch:`
* `entity.setAbsPos(entity_id,:x:,:y:,:z:)`
  * Description: Move the specified entity to the given absolute[^2] position.
* `entity.getAbsPos(entity_id)`
  * Description: Retrieve the given entity's absolute[^2] position.
  * Output: `:x:,:y:,:z:`
* `entity.events.block.hits(entity_id)`
  * Description: Retrieve all queued block hit events produced by the specified entity.
  * Output: See `events.block.hits`.
* `events.chat.posts()`
  * Description: Retrieve all queued chat events.
  * Output: List of `entity_id,{message}`
    * When compatibility mode is disabled, this will also include non-chat messages denoted with an entity ID of `-1`.
* `entity.events.chat.posts(entity_id)`
  * Description: Retrieve all queued chat events produced by the specified entity.
  * Output: See above.
* `events.projectile.hits()`
  * Description: Retrieve all queued projectile hit events.
  * Output: List of <code>x,y,z<b>,1,shooter_entity_name,\[target_entity_name\]</b><ins>,shooter_entity_id,target_entity_id</ins></code>
    * If the projectile hit a block, `target_entity_id` will be `-1` and `target_entity_name` will be blank.
* `entity.events.projectile.hits(entity_id)`
  * Description: Retrieve all queued projectile hit events produced by the specified entity.
  * Output: See above.
* `entity.events.clear(entity_id)`
  * Description: Clear all queued events produced by the specified entity.

### Reborn-Specific
* `reborn.disableCompatMode()`
  * Description: Disable the compatibility mode.
  * Output: `reborn_version`
* `reborn.enableCompatMode()`
  * Description: Re-enable the compatibility mode.
* `world.getSign(x,y,z)`
  * Description: Retrieve the text of the given sign.
  * Output: List of `{line}`
* `entity.getType(entity_id)`
  * Description: Check the type of the given entity.
  * Output: `entity_type_id`

[^1]: If the ID is `-1`, it will match all entities.
[^2]: The API normally applies an offset to all coordinates, these commands use the raw data.