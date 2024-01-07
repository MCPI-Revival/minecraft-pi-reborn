project(symbols)

# Definition Files
set(SRC
    src/LeafTile.def
    src/ServerPlayer.def
    src/Tile.def
    src/Material.def
    src/RakNetInstance.def
    src/LiquidTile.def
    src/LoginPacket.def
    src/ChestTileEntity.def
    src/InBedScreen.def
    src/LevelSource.def
    src/TallGrass.def
    src/Player.def
    src/ImageButton.def
    src/Strings.def
    src/ExternalFileLevelStorageSource.def
    src/TileEntity.def
    src/ItemRenderer.def
    src/ModelPart.def
    src/Biome.def
    src/SimpleChooseLevelScreen.def
    src/PauseScreen.def
    src/Tesselator.def
    src/AABB.def
    src/AppPlatform.def
    src/Minecraft.def
    src/OptionsFile.def
    src/EntityRenderer.def
    src/RakNet_RakString_SharedString.def
    src/ChunkSource.def
    src/Vec3.def
    src/NetEventCallback.def
    src/PathfinderMob.def
    src/Level.def
    src/RakNet_RakString.def
    src/StemTile.def
    src/LevelRenderer.def
    src/FillingContainer.def
    src/HumanoidModel.def
    src/TripodCameraRenderer.def
    src/FurnaceScreen.def
    src/HitResult.def
    src/Container.def
    src/PerfRenderer.def
    src/ParticleEngine.def
    src/PlayerRenderer.def
    src/GameMode.def
    src/Font.def
    src/RemotePlayer.def
    src/EntityRenderDispatcher.def
    src/Texture.def
    src/MobRenderer.def
    src/LargeFeature.def
    src/ContainerMenu.def
    src/Textures.def
    src/FurnaceTileEntity.def
    src/TileRenderer.def
    src/GrassTile.def
    src/WorkbenchScreen.def
    src/Packet.def
    src/RandomLevelSource.def
    src/SelectWorldScreen.def
    src/AuxDataTileItem.def
    src/OptionButton.def
    src/Touch_IngameBlockSelectionScreen.def
    src/OptionsPane.def
    src/HeavyTile.def
    src/Options.def
    src/AppPlatform_linux.def
    src/StartGamePacket.def
    src/IBuildInput.def
    src/ItemInstance.def
    src/SoundEngine.def
    src/GuiComponent.def
    src/NinecraftApp.def
    src/Mob.def
    src/MouseBuildInput.def
    src/FurnaceRecipes.def
    src/Recipes.def
    src/Button.def
    src/ChatPacket.def
    src/LevelData.def
    src/ServerSideNetworkHandler.def
    src/LargeCaveFeature.def
    src/Screen.def
    src/CreatorMode.def
    src/HumanoidMobRenderer.def
    src/OptionsScreen.def
    src/Gui.def
    src/Recipes_Type.def
    src/Inventory.def
    src/Common.def
    src/CommandServer.def
    src/RakNet_SystemAddress.def
    src/LevelSettings.def
    src/ServerLevel.def
    src/Item.def
    src/extra.h
    src/Entity.def
    src/RakNet_RakNetGUID.def
    src/RakNet_RakPeer.def
    src/Config.def
    src/ConnectedClient.def
    src/Mouse.def
    src/GameRenderer.def
    src/SurvivalMode.def
    src/AppPlatform_readAssetFile_return_value.def
    src/TextEditScreen.def
    src/StartMenuScreen.def
    src/ProgressScreen.def
    src/LocalPlayer.def
    src/Touch_SelectWorldScreen.def
    src/TileItem.def
    src/Tile_SoundType.def
    src/Options_Option.def
    src/LevelStorageSource.def
    src/SignTileEntity.def
    src/AgableMob.def
    src/PrimedTnt.def
    src/ItemEntity.def
    src/Arrow.def
    src/OffsetPosTranslator.def
    src/EntityFactory.def
    src/MobFactory.def
    src/Animal.def
    src/Sheep.def
    src/PingedCompatibleServer.def
)
# Resolve Definition Files
set(RESOLVED_SRC "")
foreach(FILE IN LISTS SRC)
    list(APPEND RESOLVED_SRC "${CMAKE_CURRENT_SOURCE_DIR}/${FILE}")
endforeach()

# Directories
function(set_and_mkdir name dir)
    set("${name}" "${dir}" PARENT_SCOPE)
    file(MAKE_DIRECTORY "${dir}")
endfunction()
set_and_mkdir(GENERATED_DIR "${CMAKE_CURRENT_BINARY_DIR}/generated")
set_and_mkdir(INCLUDE_OUTPUT_DIR "${GENERATED_DIR}/include")
set_and_mkdir(HEADER_OUTPUT_DIR "${INCLUDE_OUTPUT_DIR}/symbols")
set_and_mkdir(SRC_OUTPUT_DIR "${GENERATED_DIR}/src")

# Files
set(HEADER_OUTPUT_FILE "${HEADER_OUTPUT_DIR}/minecraft.h")
set(SRC_OUTPUT_FILE "${SRC_OUTPUT_DIR}/minecraft.cpp")

# Generate
add_custom_command(
    OUTPUT "${SRC_OUTPUT_FILE}" "${HEADER_OUTPUT_FILE}"
    DEPENDS ${RESOLVED_SRC}
    COMMAND npm start --silent -- "${SRC_OUTPUT_FILE}" "${HEADER_OUTPUT_FILE}" ${RESOLVED_SRC}
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../dependencies/symbol-processor/src"
    VERBATIM
)

# Build
add_library(symbols SHARED "${SRC_OUTPUT_FILE}" "${HEADER_OUTPUT_FILE}")

# Show In IDE
add_custom_target(symbols-src
    DEPENDS "${SRC_OUTPUT_FILE}" "${HEADER_OUTPUT_FILE}"
    SOURCES ${RESOLVED_SRC}
)

# Include Directory
target_include_directories(
    symbols
    PUBLIC
    "$<BUILD_INTERFACE:${INCLUDE_OUTPUT_DIR}>"
    "$<INSTALL_INTERFACE:${MCPI_SDK_INCLUDE_DIR}/symbols>"
)

# Disable C++11 String ABI
target_compile_definitions(symbols PUBLIC -D_GLIBCXX_USE_CXX11_ABI=0)

# Install
install(TARGETS symbols DESTINATION "${MCPI_LIB_DIR}")
# SDK
install(TARGETS symbols EXPORT sdk DESTINATION "${MCPI_SDK_LIB_DIR}")
install(DIRECTORY "${INCLUDE_OUTPUT_DIR}/" DESTINATION "${MCPI_SDK_INCLUDE_DIR}/symbols")
