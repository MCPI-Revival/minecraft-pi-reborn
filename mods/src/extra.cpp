#include <string>
#include <fstream>
#include <streambuf>
#include <vector>

#include <unistd.h>

#include <GLES/gl.h>

#include <libcore/libcore.h>

#include "extra.h"
#include "cxx11_util.h"
#include "screenshot/screenshot.h"

#include "minecraft.h"

#include <cstdio>

extern "C" {
    // Read Asset File
    static cxx11_string AppPlatform_readAssetFile_injection(__attribute__((unused)) unsigned char *app_platform, std::string const& path) {
        std::string full_path("./data/");
        full_path.append(path);
        std::ifstream stream(full_path);
        std::string str((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());
        return create_cxx11_string(str.c_str());
    }

    // Take Screenshot Using TripodCamera
    static void AppPlatform_linux_saveScreenshot_injection(__attribute__((unused)) unsigned char *app_platform, __attribute__((unused)) std::string const& param1, __attribute__((unused)) std::string const& param_2) {
        take_screenshot();
    }

    // Open Sign Screen
    static void LocalPlayer_openTextEdit_injection(unsigned char *local_player, unsigned char *sign) {
        if (*(int *)(sign + 0x18) == 4) {
            unsigned char *minecraft = *(unsigned char **) (local_player + 0xc90);
            unsigned char *screen = (unsigned char *) ::operator new(0xd0);
            screen = (*TextEditScreen)(screen, sign);
            (*Minecraft_setScreen)(minecraft, screen);
        }
    }

    #define BACKSPACE_KEY 8

    static int is_valid_key(char key) {
        return (key >= 32 && key <= 126) || key == BACKSPACE_KEY;
    }

    // Store Text Input
    std::vector<char> input;
    void extra_key_press(char key) {
        if (is_valid_key(key)) {
            input.push_back(key);
        }
    }
    void extra_clear_input() {
        input.clear();
    }

    // Handle Text Input
    static void TextEditScreen_updateEvents_injection(unsigned char *screen) {
        // Call Original Method
        (*Screen_updateEvents)(screen);

        if (*(char *)(screen + 4) == '\0') {
            uint32_t vtable = *((uint32_t *) screen);
            for (char key : input) {
                if (key == BACKSPACE_KEY) {
                    // Handle Backspace
                    (*(Screen_keyPressed_t *) (vtable + 0x6c))(screen, BACKSPACE_KEY);
                } else {
                    // Handle Nrmal Key
                    (*(Screen_keyboardNewChar_t *) (vtable + 0x70))(screen, key);
                }
            }
        }
        extra_clear_input();
    }

    #define ITEM_INSTANCE_SIZE 0xc

    static void inventory_add_item(unsigned char *inventory, unsigned char *item, bool is_tile) {
        unsigned char *item_instance = (unsigned char *) ::operator new(ITEM_INSTANCE_SIZE);
        item_instance = (*(is_tile ? ItemInstance_constructor_tile : ItemInstance_constructor_item))(item_instance, item);
        (*FillingContainer_addItem)(inventory, item_instance);
    }

    static int32_t Inventory_setupDefault_FillingContainer_addItem_call_injection(unsigned char *filling_container, unsigned char *item_instance) {
        // Call Original
        int32_t ret = (*FillingContainer_addItem)(filling_container, item_instance);

        // Add Items
        inventory_add_item(filling_container, *Item_flintAndSteel, false);
        inventory_add_item(filling_container, *Item_snowball, false);
        inventory_add_item(filling_container, *Item_egg, false);
        inventory_add_item(filling_container, *Item_shears, false);
        for (int i = 0; i < 15; i++) {
            unsigned char *item_instance = (unsigned char *) ::operator new(0xc);
            item_instance = (*ItemInstance_constructor_item_extra)(item_instance, *Item_dye_powder, 1, i);
            (*FillingContainer_addItem)(filling_container, item_instance);
        }
        inventory_add_item(filling_container, *Item_camera, false);
        // Add Tiles
        inventory_add_item(filling_container, *Tile_water, true);
        inventory_add_item(filling_container, *Tile_lava, true);
        inventory_add_item(filling_container, *Tile_calmWater, true);
        inventory_add_item(filling_container, *Tile_calmLava, true);
        inventory_add_item(filling_container, *Tile_glowingObsidian, true);
        inventory_add_item(filling_container, *Tile_web, true);
        inventory_add_item(filling_container, *Tile_topSnow, true);
        inventory_add_item(filling_container, *Tile_ice, true);
        inventory_add_item(filling_container, *Tile_invisible_bedrock, true);

        return ret;
    }

    static void Minecraft_tick_injection(unsigned char *minecraft, int32_t param_1, int32_t param_2) {
        // Call Original Method
        (*Minecraft_tick)(minecraft, param_1, param_2);

        // Tick Dynamic Textures
        unsigned char *textures = *(unsigned char **) (minecraft + 0x164);
        if (textures != NULL) {
            (*Textures_tick)(textures, true);
        }
    }

    // Get Minecraft From Screen
    static unsigned char *get_minecraft_from_screen(unsigned char *screen) {
        return *(unsigned char **) (screen + 0x14);
    }

    // Redirect Create World Button To SimpleLevelChooseScreen
    #define WORLD_NAME "world"
    #define SIMPLE_LEVEL_CHOOSE_SCREEN_SIZE 0x68
    static void SelectWorldScreen_tick_injection(unsigned char *screen) {
        bool create_world = *(bool *) (screen + 0xfc);
        if (create_world) {
            // Get New World Name
            std::string new_name;
            (*SelectWorldScreen_getUniqueLevelName)(new_name, screen, WORLD_NAME);
            // Create SimpleLevelChooseScreen
            unsigned char *new_screen = (unsigned char *) ::operator new(SIMPLE_LEVEL_CHOOSE_SCREEN_SIZE);
            (*SimpleChooseLevelScreen)(new_screen, new_name);
            // Set Screen
            unsigned char *minecraft = get_minecraft_from_screen(screen);
            (*Minecraft_setScreen)(minecraft, new_screen);
            // Finish
            *(bool *) (screen + 0xf9) = true;
        } else {
            (*SelectWorldScreen_tick)(screen);
        }
    }
    static void Touch_SelectWorldScreen_tick_injection(unsigned char *screen) {
        bool create_world = *(bool *) (screen + 0x154);
        if (create_world) {
            // Get New World Name
            std::string new_name;
            (*Touch_SelectWorldScreen_getUniqueLevelName)(new_name, screen, WORLD_NAME);
            // Create SimpleLevelChooseScreen
            unsigned char *new_screen = (unsigned char *) ::operator new(SIMPLE_LEVEL_CHOOSE_SCREEN_SIZE);
            (*SimpleChooseLevelScreen)(new_screen, new_name);
            // Set Screen
            unsigned char *minecraft = get_minecraft_from_screen(screen);
            (*Minecraft_setScreen)(minecraft, new_screen);
            // Finish
            *(bool *) (screen + 0x151) = true;
        } else {
            (*Touch_SelectWorldScreen_tick)(screen);
        }
    }

    // Enable TripodCameraRenderer
    static unsigned char *EntityRenderDispatcher_injection(unsigned char *dispatcher) {
        // Call Original Method
        (*EntityRenderDispatcher)(dispatcher);

        // Register TripodCameraRenderer
        unsigned char *renderer = (unsigned char *) ::operator new(0x193);
        (*TripodCameraRenderer)(renderer);
        (*EntityRenderDispatcher_assign)(dispatcher, (unsigned char) 0x5, renderer);

        return dispatcher;
    }
    // Display Smoke From TripodCamera Higher
    static void Level_addParticle_injection(unsigned char *level, std::string const& particle, float x, float y, float z, float deltaX, float deltaY, float deltaZ, int count) {
        // Call Original Method
        (*Level_addParticle)(level, particle, x, y + 0.5, z, deltaX, deltaY, deltaZ, count);
    }

    // Fix Grass And Leaves Inventory Rendering When The gui_blocks Atlas Is Disabled
    static float ItemRenderer_renderGuiItemCorrect_injection(unsigned char *font, unsigned char *textures, unsigned char *item_instance, int32_t param_1, int32_t param_2) {
        int32_t leaves_id = *(int32_t *) (*Tile_leaves + 0x8);
        int32_t grass_id = *(int32_t *) (*Tile_grass + 0x8);
        // Replace Rendered Item With Carried Variant
        unsigned char *carried_item_instance = NULL;
        if (item_instance != NULL) {
            int32_t id = *(int32_t *) (item_instance + 0x4);
            int32_t count = *(int32_t *) item_instance;
            int32_t auxilary = *(int32_t *) (item_instance + 0x8);
            if (id == leaves_id) {
                carried_item_instance = (unsigned char *) ::operator new(ITEM_INSTANCE_SIZE);
                (*ItemInstance_constructor_title_extra)(carried_item_instance, *Tile_leaves_carried, count, auxilary);
            } else if (id == grass_id) {
                carried_item_instance = (unsigned char *) ::operator new(ITEM_INSTANCE_SIZE);
                (*ItemInstance_constructor_title_extra)(carried_item_instance, *Tile_grass_carried, count, auxilary);
            }
        }
        // Fix Toolbar Rendering
        GLboolean depth_test_was_enabled = glIsEnabled(GL_DEPTH_TEST);
        glDisable(GL_DEPTH_TEST);
        // Call Original Method
        float ret = (*ItemRenderer_renderGuiItemCorrect)(font, textures, carried_item_instance != NULL ? carried_item_instance : item_instance, param_1, param_2);
        // Revert GL State Changes
        if (depth_test_was_enabled) {
            glEnable(GL_DEPTH_TEST);
        }
        // Free Carried Item Instance Variant
        if (carried_item_instance != NULL) {
            ::operator delete(carried_item_instance);
        }
        // Return
        return ret;
    }

    // Render Selected Item Text
    static void Gui_renderChatMessages_injection(unsigned char *gui, int32_t param_1, uint32_t param_2, bool param_3, unsigned char *font) {
        // Call Original Method
        (*Gui_renderChatMessages)(gui, param_1, param_2, param_3, font);
        // Calculate Selected Item Text Scale
        unsigned char *minecraft = *(unsigned char **) (gui + 0x9f4);
        int32_t screen_width = *(int32_t *) (minecraft + 0x20);
        float scale = ((float) screen_width) * *InvGuiScale;
        // Render Selected Item Text
        (*Gui_renderOnSelectItemNameText)(gui, (int32_t) scale, font, param_1 - 0x13);
    }
    // Reset Selected Item Text Timer On Slot Select
    static bool reset_selected_item_text_timer = false;
    static void Gui_tick_injection(unsigned char *gui) {
        // Call Original Method
        (*Gui_tick)(gui);
        // Handle Reset
        float *selected_item_text_timer = (float *) (gui + 0x9fc);
        if (reset_selected_item_text_timer) {
            // Reset
            *selected_item_text_timer = 0;
            reset_selected_item_text_timer = false;
        }
    }
    // Trigger Reset Selected Item Text Timer On Slot Select
    static void Inventory_selectSlot_injection(unsigned char *inventory, int32_t slot) {
        // Call Original Method
        (*Inventory_selectSlot)(inventory, slot);
        // Trigger Reset Selected Item Text Timer
        reset_selected_item_text_timer = true;
    }

    __attribute((constructor)) static void init() {
        // Implement AppPlatform::readAssetFile So Translations Work
        overwrite((void *) AppPlatform_readAssetFile, (void *) AppPlatform_readAssetFile_injection);
        // Implement AppPlatform_linux::saveScreenshot So Cameras Work
        patch_address(AppPlatform_linux_saveScreenshot_vtable_addr, (void *) AppPlatform_linux_saveScreenshot_injection);

        // Enable TripodCameraRenderer
        overwrite_calls((void *) EntityRenderDispatcher, (void *) EntityRenderDispatcher_injection);
        // Display Smoke From TripodCamera Higher
        overwrite_call((void *) 0x87dc4, (void *) Level_addParticle_injection);

        if (extra_has_feature("Fix Sign Placement")) {
            // Fix Signs
            patch_address(LocalPlayer_openTextEdit_vtable_addr, (void *) LocalPlayer_openTextEdit_injection);
            patch_address(TextEditScreen_updateEvents_vtable_addr, (void *) TextEditScreen_updateEvents_injection);
        }

        if (extra_has_feature("Expand Creative Inventory")) {
            // Add Extra Items To Creative Inventory (Only Replace Specific Function Call)
            overwrite_call((void *) 0x8e0fc, (void *) Inventory_setupDefault_FillingContainer_addItem_call_injection);
        }

        if (extra_has_feature("Animated Water")) {
            // Tick Dynamic Textures (Animated Water)
            overwrite_calls((void *) Minecraft_tick, (void *) Minecraft_tick_injection);
        }

        // Hijack Create World Button
        patch_address(SelectWorldScreen_tick_vtable_addr, (void *) SelectWorldScreen_tick_injection);
        patch_address(Touch_SelectWorldScreen_tick_vtable_addr, (void *) Touch_SelectWorldScreen_tick_injection);
        // Make The SimpleChooseLevelScreen Back Button Go To SelectWorldScreen Instead Of StartMenuScreen
        unsigned char simple_choose_level_screen_back_button_patch[4] = {0x05, 0x10, 0xa0, 0xe3};
        patch((void *) 0x31144, simple_choose_level_screen_back_button_patch);

        if (extra_has_feature("Disable gui_blocks Atlas")) {
            // Disable gui_blocks Atlas Which Contains Pre-Rendered Textures For Blocks In The Inventory
            unsigned char disable_gui_blocks_atlas_patch[4] = {0x00, 0xf0, 0x20, 0xe3};
            patch((void *) 0x63c2c, disable_gui_blocks_atlas_patch);
            // Fix Grass And Leaves Inventory Rendering When The gui_blocks Atlas Is Disabled
            overwrite_calls((void *) ItemRenderer_renderGuiItemCorrect, (void *) ItemRenderer_renderGuiItemCorrect_injection);
        }

        // Fix Selected Item Text
        overwrite_calls((void *) Gui_renderChatMessages, (void *) Gui_renderChatMessages_injection);
        overwrite_calls((void *) Gui_tick, (void *) Gui_tick_injection);
        overwrite_calls((void *) Inventory_selectSlot, (void *) Inventory_selectSlot_injection);
    }
}
