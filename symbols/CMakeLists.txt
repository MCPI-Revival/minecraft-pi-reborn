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
    src/game/NinecraftApp.def
    src/game/GameRenderer.def
    src/network/RakNetInstance.def
    src/network/PingedCompatibleServer.def
    src/network/NetEventCallback.def
    src/network/raknet/RakNet_RakString_SharedString.def
    src/network/raknet/RakNet_RakString.def
    src/network/raknet/RakNet_SystemAddress.def
    src/network/raknet/RakNet_StartupResult.h
    src/network/raknet/RakNet_RakNetGUID.def
    src/network/raknet/RakNet_RakPeer.def
    src/network/raknet/RakNet_BitStream.def
    src/network/ServerSideNetworkHandler.def
    src/network/packet/LoginPacket.def
    src/network/packet/PlayerEquipmentPacket.def
    src/network/packet/SignUpdatePacket.def
    src/network/packet/Packet.def
    src/network/packet/StartGamePacket.def
    src/network/packet/ChatPacket.def
    src/entity/EntityFactory.def
    src/entity/PrimedTnt.def
    src/entity/CameraEntity.def
    src/entity/EntityRenderer.def
    src/entity/ItemSpriteRenderer.def
    src/entity/PathfinderMob.def
    src/entity/HumanoidModel.def
    src/entity/TripodCameraRenderer.def
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
    src/entity/Throwable.def
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
    src/level/renderer/LevelRenderer.def
    src/level/LevelStorageSource.def
    src/level/renderer/ParticleEngine.def
    src/level/RandomLevelSource.def
    src/level/PerformanceTestChunkSource.def
    src/level/LevelData.def
    src/level/LevelSettings.def
    src/level/ServerLevel.def
    src/level/Dimension.def
    src/level/MultiPlayerLevel.def
    src/level/LevelSummary.def
    src/level/DistanceChunkSorter.def
    src/level/renderer/Chunk.def
    src/level/LevelStorage.def
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
    src/gui/screens/ProgressScreen.def
    src/gui/screens/Touch_SelectWorldScreen.def
    src/gui/screens/PaneCraftingScreen.def
    src/gui/screens/ScreenChooser.def
    src/gui/Font.def
    src/gui/components/ImageButton.def
    src/gui/components/OptionButton.def
    src/gui/components/OptionsPane.def
    src/gui/components/GuiComponent.def
    src/gui/components/Button.def
    src/gui/Gui.def
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
    src/misc/Strings.def
    src/misc/I18n.def
    src/misc/SimpleFoodData.def
    src/entity/ModelPart.def
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
function(set_and_mkdir name dir)
    set("${name}" "${dir}" PARENT_SCOPE)
    file(MAKE_DIRECTORY "${dir}")
endfunction()
set_and_mkdir(GENERATED_DIR "${CMAKE_CURRENT_BINARY_DIR}/generated")
set_and_mkdir(INCLUDE_OUTPUT_DIR "${GENERATED_DIR}/include")
set_and_mkdir(HEADER_OUTPUT_DIR "${INCLUDE_OUTPUT_DIR}/symbols")
set_and_mkdir(SRC_OUTPUT_DIR "${GENERATED_DIR}/src")

# Generated Header File
set(HEADER_OUTPUT_FILE "${HEADER_OUTPUT_DIR}/minecraft.h")

# Generated Source Files
set(SRC_OUTPUT_FILES "")
foreach(FILE IN LISTS SRC)
    get_filename_component(FILE_EXT "${FILE}" EXT)
    if(FILE_EXT STREQUAL ".def")
        get_filename_component(FILE "${FILE}" NAME_WE)
        list(APPEND SRC_OUTPUT_FILES "${SRC_OUTPUT_DIR}/${FILE}.cpp")
    endif()
endforeach()

# Generate
set(INPUT_FILES "")
foreach(FILE IN LISTS SRC)
    list(APPEND INPUT_FILES "${CMAKE_CURRENT_SOURCE_DIR}/${FILE}")
endforeach()
set(OUTPUT_FILES ${SRC_OUTPUT_FILES} "${HEADER_OUTPUT_FILE}")
add_custom_command(
    OUTPUT ${OUTPUT_FILES}
    DEPENDS ${INPUT_FILES}
    COMMAND npm start --silent -- "${SRC_OUTPUT_DIR}" "${HEADER_OUTPUT_FILE}" ${INPUT_FILES}
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../dependencies/symbol-processor/src"
    VERBATIM
)

# Build
add_library(symbols SHARED ${OUTPUT_FILES})

# Speed Up Compilation
#target_precompile_headers(symbols PRIVATE "${HEADER_OUTPUT_FILE}")

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
# Fix Warning
target_compile_options(symbols PUBLIC -Wno-missing-field-initializers)

# Install
install(TARGETS symbols DESTINATION "${MCPI_LIB_DIR}")
# SDK
install(TARGETS symbols EXPORT sdk DESTINATION "${MCPI_SDK_LIB_DIR}")
install(DIRECTORY "${INCLUDE_OUTPUT_DIR}/" DESTINATION "${MCPI_SDK_INCLUDE_DIR}/symbols")
