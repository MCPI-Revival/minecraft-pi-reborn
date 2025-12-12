#include <symbols/Tile.h>
#include <symbols/Player.h>
#include <symbols/Level.h>
#include <symbols/Material.h>
#include <symbols/FillingContainer.h>
#include <symbols/ItemInstance.h>
#include <symbols/I18n.h>

#include <mods/misc/misc.h>

// Custom Tile
struct CustomBlock final : CustomTile {
    // Constructor
    CustomBlock(): CustomTile(252, 30, Material::glass) {}

    // On Use
    bool use(Level *level, int x, int y, int z, Player *player) override {
        if (!level->is_client_side) {
            player->displayClientMessage("Hello World!");
        }
        return true;
    }
};
static constexpr const char *description_id = "custom_block";
static Tile *block;

// Init
__attribute__((constructor)) static void init_custom_block() {
    // Construct Block
    misc_run_on_tiles_setup([]() {
        block = (new CustomBlock())->self;
        block->init();
        block->setDescriptionId(description_id);
    });
    // Add To Creative Inventory
    misc_run_on_creative_inventory_setup([](FillingContainer *inventory) {
        inventory->addItem(new ItemInstance {
            .count = 1,
            .id = block->id,
            .auxiliary = 0
        });
    });
    // Add Language String
    misc_run_on_language_setup([]() {
        I18n::_strings[std::string("tile.") + description_id + ".name"] = "Custom Block";
    });
}