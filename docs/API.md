---
gitea: none
include_toc: true
---

# API
In addition to Minecraft: Pi Edition's [builtin API](https://minecraft.wiki/w/Pi_Edition_protocol), MCPI-Reborn also implements the [RaspberryJuice API](https://github.com/zhuowei/RaspberryJuice). This page documents the API.

The API operates over a TCP socket.

Commands are formatted like `<command>(<args>)` and may return a response. The response `Fail\n` indicates an error.

It is typically hosted on port `4711`, but this can be configured.

## Compatibility Mode
By default, MCPI-Reborn runs in a "compatibility mode." This makes it completely compatible with RaspberryJuice, but limits functionality.

* **Bold** text only applies to the compatibility mode.
* <u>Underlined</u> text only applies when it is disabled.
* Text enclosed in curly braces (`{}`) is meant to be [Base64-URL](https://base64.guru/standards/base64url) encoded when the compatibility mode is disabled.

## Commands

* Arguments surrounded by square brackets (`[]`) are optional.
* Lists are delimited by `|`. For instance: `A|B|C`.

### Vanilla

### RaspberryJuice

### Reborn-Specific
* `world.getSign(x,y,z)`
  * Description: Retrieve the text of the given sign.
  * Output: `{line_1}|{line_2}|{line_3}|{line_4}`
* `entity.getType(id)`
  * Description: Check the type of the given entity.
  * Output: `entity_id`