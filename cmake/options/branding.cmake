# Author
mcpi_option(AUTHOR "Author" STRING "TheBrokenRail")
mcpi_option(AUTHOR_LONG "Author (Long Version)" STRING "${MCPI_AUTHOR} & Mojang AB")
mcpi_option(AUTHOR_ID "Author ID" STRING "com.thebrokenrail")

# App Information
mcpi_option(APP_NAME "App Name" STRING "minecraft-pi-reborn")
mcpi_option(APP_ID "App ID" STRING "${MCPI_AUTHOR_ID}.MCPIReborn")
mcpi_option(APP_TITLE "App Title" STRING "Minecraft: Pi Edition: Reborn")
mcpi_option(APP_DESCRIPTION "Package Description" STRING "Fun with Blocks")

# Skin Server
mcpi_option(SKIN_SERVER "Skin Server" STRING "https://raw.githubusercontent.com/MCPI-Revival/Skins/data")

# Discord Invite URL
mcpi_option(DISCORD_INVITE "Discord Invite URL" STRING "https://discord.gg/mcpi-revival-740287937727561779")

# Homepage
mcpi_option(REPO_HOST "Repository Host" STRING "https://gitea.thebrokenrail.com")
mcpi_option(REPO_PATH "Repository Path" STRING "minecraft-pi-reborn/minecraft-pi-reborn")
mcpi_option(REPO "Repository URL" STRING "${MCPI_REPO_HOST}/${MCPI_REPO_PATH}")

# Documentation URL
mcpi_option(DOCS "Documentation URL" STRING "${MCPI_REPO}/src/tag/${MCPI_VERSION}/docs/")
mcpi_option(CHANGELOG "Changelog URL" STRING "${MCPI_REPO}/releases/tag/${MCPI_VERSION}")