project(symbols)

# Definition Files
set(SRC
    src/game/options/Options_Option.def
    src/game/options/OptionsFile.def
    src/game/options/Options.def
    src/game/Minecraft.def
    src/game/mode/GameMode.def
    src/game/mode/CreatorMode.def
    src/game/mode/SurvivalMode.def
    src/game/mode/creator/Creator.def
    src/game/mode/creator/EventList_Item.def
    src/game/mode/creator/EventList_TileEvent.def
    src/game/mode/creator/ICreator.def
    src/game/mode/creator/TileEvent.def
    src/game/NinecraftApp.def
    src/game/GameRenderer.def
    src/network/RakNetInstance.def
    src/network/PingedCompatibleServer.def
    src/network/NetEventCallback.def
    src/network/raknet/RakNet_RakString_SharedString.def
    src/network/raknet/RakNet_RakString.def
    src/network/raknet/RakNet_SystemAddress.def
    src/network/raknet/RakNet.h
    src/network/raknet/RakNet_RakNetGUID.def
    src/network/raknet/RakNet_RakPeer.def
    src/network/raknet/RakNet_BitStream.def
    src/network/raknet/RakNet_AddressOrGUID.def
    src/network/ServerSideNetworkHandler.def
    src/network/ClientSideNetworkHandler.def
    src/network/packet/LoginPacket.def
    src/network/packet/PlayerEquipmentPacket.def
    src/network/packet/SignUpdatePacket.def
    src/network/packet/Packet.def
    src/network/packet/StartGamePacket.def
    src/network/packet/ChatPacket.def
    src/network/packet/PlayerActionPacket.def
    src/network/packet/MovePlayerPacket.def
    src/network/packet/SetEntityMotionPacket.def
    src/network/packet/ChunkDataPacket.def
    src/network/packet/RequestChunkPacket.def
    src/network/packet/UpdateBlockPacket.def
    src/network/packet/ReadyPacket.def
    src/network/packet/SendInventoryPacket.def
    src/network/packet/MessagePacket.def
    src/network/packet/SetSpawnPositionPacket.def
    src/network/packet/ContainerSetContentPacket.def
    src/network/packet/UseItemPacket.def
    src/entity/EntityFactory.def
    src/entity/PrimedTnt.def
    src/entity/CameraEntity.def
    src/entity/EntityRenderer.def
    src/entity/ItemSpriteRenderer.def
    src/entity/PathfinderMob.def
    src/entity/model/HumanoidModel.def
    src/entity/TripodCameraRenderer.def
    src/entity/TripodCamera.def
    src/entity/MobFactory.def
    src/entity/EntityRenderDispatcher.def
    src/entity/MobRenderer.def
    src/entity/animal/AgableMob.def
    src/entity/animal/Animal.def
    src/entity/animal/Cow.def
    src/entity/animal/Chicken.def
    src/entity/animal/Pig.def
    src/entity/animal/Sheep.def
    src/entity/monster/Creeper.def
    src/entity/monster/Monster.def
    src/entity/monster/PigZombie.def
    src/entity/monster/Skeleton.def
    src/entity/monster/Spider.def
    src/entity/monster/Zombie.def
    src/entity/Mob.def
    src/entity/player/ServerPlayer.def
    src/entity/player/Player.def
    src/entity/player/PlayerRenderer.def
    src/entity/player/RemotePlayer.def
    src/entity/player/LocalPlayer.def
    src/entity/HumanoidMobRenderer.def
    src/entity/Entity.def
    src/entity/ItemEntity.def
    src/entity/Arrow.def
    src/entity/ArrowRenderer.def
    src/entity/Throwable.def
    src/entity/Particle.def
    src/entity/painting/Painting.def
    src/entity/painting/PaintingRenderer.def
    src/entity/painting/HangingEntity.def
    src/entity/painting/Motive.def
    src/entity/FallingTile.def
    src/level/container/FillingContainer.def
    src/level/container/Container.def
    src/level/container/ContainerMenu.def
    src/level/container/Inventory.def
    src/level/feature/Feature.def
    src/level/feature/LargeFeature.def
    src/level/feature/LargeCaveFeature.def
    src/level/Material.def
    src/level/LevelSource.def
    src/level/ExternalFileLevelStorageSource.def
    src/level/Biome.def
    src/level/LevelChunk.def
    src/level/ChunkSource.def
    src/level/ChunkCache.def
    src/level/ChunkStorage.def
    src/level/LightLayer.def
    src/level/Level.def
    src/level/LevelListener.def
    src/level/renderer/LevelRenderer.def
    src/level/LevelStorageSource.def
    src/level/renderer/ParticleEngine.def
    src/level/RandomLevelSource.def
    src/level/PerformanceTestChunkSource.def
    src/level/LevelData.def
    src/level/LevelSettings.def
    src/level/ServerLevel.def
    src/level/CreatorLevel.def
    src/level/Dimension.def
    src/level/MultiPlayerLevel.def
    src/level/LevelSummary.def
    src/level/DistanceChunkSorter.def
    src/level/renderer/Chunk.def
    src/level/LevelStorage.def
    src/level/BiomeSource.def
    src/level/Region.def
    src/level/DataLayer.def
    src/item/ItemRenderer.def
    src/item/ItemInHandRenderer.def
    src/item/AuxDataTileItem.def
    src/item/ItemInstance.def
    src/item/Item.def
    src/item/ArmorMaterial.def
    src/item/ArmorItem.def
    src/item/TileItem.def
    src/item/TilePlanterItem.def
    src/item/FoodItem.def
    src/item/DiggerItem.def
    src/api/OffsetPosTranslator.def
    src/api/CommandServer.def
    src/api/ConnectedClient.def
    src/gui/screens/InBedScreen.def
    src/gui/screens/SimpleChooseLevelScreen.def
    src/gui/screens/PauseScreen.def
    src/gui/screens/FurnaceScreen.def
    src/gui/screens/WorkbenchScreen.def
    src/gui/screens/SelectWorldScreen.def
    src/gui/screens/Touch_IngameBlockSelectionScreen.def
    src/gui/screens/Screen.def
    src/gui/screens/OptionsScreen.def
    src/gui/screens/TextEditScreen.def
    src/gui/screens/StartMenuScreen.def
    src/gui/screens/Touch_StartMenuScreen.def
    src/gui/screens/ProgressScreen.def
    src/gui/screens/Touch_SelectWorldScreen.def
    src/gui/screens/PaneCraftingScreen.def
    src/gui/screens/ScreenChooser.def
    src/gui/screens/ArmorScreen.def
    src/gui/screens/DisconnectionScreen.def
    src/gui/Font.def
    src/gui/components/ImageButton.def
    src/gui/components/OptionButton.def
    src/gui/components/OptionsPane.def
    src/gui/components/GuiComponent.def
    src/gui/components/Button.def
    src/gui/Gui.def
    src/gui/GuiMessage.def
    src/gui/components/IntRectangle.def
    src/gui/components/RectangleArea.def
    src/gui/components/ScrollingPane.def
    src/gui/components/Touch_TButton.def
    src/app-platform/AppPlatform.def
    src/app-platform/AppPlatform_linux.def
    src/app-platform/AppPlatform_readAssetFile_return_value.def
    src/tile/LeafTile.def
    src/tile/DoorTile.def
    src/tile/Tile.def
    src/tile/LiquidTile.def
    src/tile/TallGrass.def
    src/tile/entity/TileEntity.def
    src/tile/entity/ChestTileEntity.def
    src/tile/entity/FurnaceTileEntity.def
    src/tile/entity/SignTileEntity.def
    src/tile/entity/TileEntityFactory.def
    src/tile/entity/TileEntityRenderer.def
    src/tile/entity/TileEntityRenderDispatcher.def
    src/tile/StemTile.def
    src/tile/Tile_SoundType.def
    src/tile/TileRenderer.def
    src/tile/CarriedTile.def
    src/tile/GrassTile.def
    src/tile/HeavyTile.def
    src/tile/EntityTile.def
    src/tile/Bush.def
    src/tile/CropTile.def
    src/tile/ChestTile.def
    src/tile/TorchTile.def
    src/misc/Strings.def
    src/misc/I18n.def
    src/misc/SimpleFoodData.def
    src/entity/model/ModelPart.def
    src/entity/model/PolygonQuad.def
    src/entity/model/VertexPT.def
    src/misc/Tesselator.def
    src/misc/AABB.def
    src/misc/Vec3.def
    src/misc/HitResult.def
    src/misc/PerfRenderer.def
    src/level/renderer/RenderList.def
    src/level/renderer/RenderChunk.def
    src/textures/Texture.def
    src/textures/Textures.def
    src/textures/DynamicTexture.def
    src/misc/SoundEngine.def
    src/misc/Common.def
    src/misc/Config.def
    src/misc/Random.def
    src/misc/Mth.def
    src/misc/Util.def
    src/misc/Tag.def
    src/misc/CompoundTag.def
    src/misc/CThread.def
    src/misc/Pos.def
    src/input/IMoveInput.def
    src/input/IBuildInput.def
    src/input/MouseBuildInput.def
    src/input/Mouse.def
    src/input/Keyboard.def
    src/recipes/FurnaceRecipes.def
    src/recipes/Recipes.def
    src/recipes/Recipes_Type.def
    src/recipes/ReqItem.def
    src/recipes/CItem.def
)

# Directories
set_and_mkdir(GENERATED_DIR "${CMAKE_CURRENT_BINARY_DIR}/generated")
set_and_mkdir(INCLUDE_OUTPUT_DIR "${GENERATED_DIR}/include")
set_and_mkdir(HEADER_OUTPUT_DIR "${INCLUDE_OUTPUT_DIR}/symbols")
set_and_mkdir(SRC_OUTPUT_DIR "${GENERATED_DIR}/src")

# Generated Header File
set(HEADER_OUTPUT_FILE "${HEADER_OUTPUT_DIR}/minecraft.h")

# Generated Source Files
set(SRC_OUTPUT_FILES "")
foreach(file IN LISTS SRC)
    cmake_path(GET file EXTENSION file_ext)
    if(file_ext STREQUAL ".def")
        cmake_path(GET file STEM file)
        list(APPEND SRC_OUTPUT_FILES "${SRC_OUTPUT_DIR}/${file}.cpp")
    endif()
endforeach()

# Generate
set(INPUT_FILES "")
foreach(file IN LISTS SRC)
    list(APPEND INPUT_FILES "${CMAKE_CURRENT_SOURCE_DIR}/${file}")
endforeach()
set(OUTPUT_FILES ${SRC_OUTPUT_FILES} "${HEADER_OUTPUT_FILE}")
add_custom_command(OUTPUT ${OUTPUT_FILES}
    DEPENDS ${INPUT_FILES}
    COMMAND "npm"
    ARGS "start" "--silent" "--" "${SRC_OUTPUT_DIR}" "${HEADER_OUTPUT_FILE}" ${INPUT_FILES}
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../dependencies/symbol-processor/src"
    VERBATIM
)

# Build
add_library(symbols SHARED ${OUTPUT_FILES})

# Include Directory
setup_header_dirs(symbols "${INCLUDE_OUTPUT_DIR}")
set(SYMBOLS_HEADER_DIR "${INCLUDE_OUTPUT_DIR}" PARENT_SCOPE)

# Install
setup_library(symbols TRUE TRUE)