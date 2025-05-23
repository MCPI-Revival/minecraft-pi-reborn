#include <symbols/minecraft.h>

#include <mods/feature/feature.h>
#include <mods/init/init.h>
#include <mods/misc/misc.h>
#include <mods/bucket/bucket.h>

#define CAKE_LEN 0.0625f
#define CAKE_SLICE_COUNT 6

// Custom Tile
struct Cake final : CustomTile {
    // Constructor
    Cake(const int id, const int texture, const Material *material): CustomTile(id, texture, material) {}

    // Textures
    int getTexture2(const int face, MCPI_UNUSED int data) override {
        if (face == 1) {
            // Top Texture
            return 121;
        } else if (face == 0) {
            // Bottom Texture
            return 124;
        }
        // Side Texture
        return 122;
    }
    int getTexture3(LevelSource *level, const int x, const int y, const int z, const int face) override {
        // Eaten Face
        if (face == 3) {
            const int data = level->getData(x, y, z);
            if (data != 0 && data < CAKE_SLICE_COUNT) {
                // Sliced Texture
                return 123;
            }
        }
        // Normal
        return getTexture2(face, 0);
    }

    // Rendering
    bool isSolidRender() override {
        // Stop From Turning Other Blocks Invisible
        return false;
    }
    int getRenderLayer() override {
        // Stop Weird Transparency Issues
        return 1;
    }
    bool isCubeShaped() override {
        return false;
    }

    // Size
    static void getShape(const int data, float &x0, float &y0, float &z0, float &x1, float &y1, float &z1) {
        x0 = CAKE_LEN;
        y0 = 0;
        z0 = x0;
        x1 = 1 - CAKE_LEN;
        y1 = 0.5f;
        z1 = x1;
        z1 -= (1 - (CAKE_LEN * 2)) * (float(data) / float(CAKE_SLICE_COUNT));
    }
    void updateShape(const int data) const {
        float x0, y0, z0;
        float x1, y1, z1;
        getShape(data, x0, y0, z0, x1, y1, z1);
        self->setShape(
            x0, y0, z0,
            x1, y1, z1
        );
    }
    void updateDefaultShape() override {
        updateShape(0);
    }
    static int getData(LevelSource *level, const int x, const int y, const int z) {
        int data = level->getData(x, y, z);
        if (data >= CAKE_SLICE_COUNT) {
            data = 0;
        }
        return data;
    }
    void updateShape(LevelSource *level, const int x, const int y, const int z) override {
        // Get Cake
        const int data = getData(level, x, y, z);
        // Get Slice Amount
        updateShape(data);
    }
    AABB *getAABB(Level *level, const int x, const int y, const int z) override {
        // Get Shape
        const int data = getData((LevelSource *) level, x, y, z);
        float x0, y0, z0;
        float x1, y1, z1;
        getShape(data, x0, y0, z0, x1, y1, z1);

        // Corner 1
        AABB *aabb = &self->aabb;
        aabb->x1 = float(x) + x0;
        aabb->y1 = float(y) + y0;
        aabb->z1 = float(z) + z0;

        // Corner 2
        aabb->x2 = float(x) + x1;
        aabb->y2 = float(y) + y1;
        aabb->z2 = float(z) + z1;

        // Return
        return aabb;
    }

    // Eating
    bool use(Level *level, const int x, const int y, const int z, Player *player) override {
        // Eat
        player->foodData.eat(3);
        // Set New Tile
        int data = level->getData(x, y, z);
        data++;
        if (data >= CAKE_SLICE_COUNT) {
            // Remove the cake, it has been completely gobbled up
            level->setTileAndData(x, y, z, 0, 0);
        } else {
            // Remove a slice
            level->setTileAndData(x, y, z, self->id, data);
        }
        return true;
    }
};
static Tile *cake = nullptr;
static TilePlanterItem *cake_item = nullptr;

// Makes The Cakes
static constexpr const char *cake_description = "cake";
static void make_cake() {
    // Construct
    cake = (new Cake(92, 122, Material::dirt))->self;

    // Set Shape
    cake->updateDefaultShape();

    // Init
    cake->init();
    cake->setDescriptionId(cake_description);
    cake->setDestroyTime(1.0f);
    cake->setExplodeable(20.0f);
    cake->category = 4;
}
static void Tile_initTiles_injection() {
    make_cake();
}
static void Item_initTiles_injection() {
    // Create Item
    cake_item = TilePlanterItem::allocate();
    ((Item *) cake_item)->constructor(98);
    cake_item->vtable = TilePlanterItem::VTable::base;
    cake_item->tile_id = cake->id;
    cake_item->setIcon(13, 1);
    cake_item->max_stack_size = 1;
    cake_item->setDescriptionId(cake_description);
}

// Add Cake To Creative Inventory
static void Inventory_setupDefault_FillingContainer_addItem_injection(FillingContainer *filling_container) {
    ItemInstance *cake_instance = new ItemInstance;
    cake_instance->count = 255;
    cake_instance->auxiliary = 0;
    cake_instance->id = cake_item->id;
    filling_container->addItem(cake_instance);
}

// Recipe (Only When Buckets Are Enabled)
static void Recipes_injection(Recipes *recipes) {
    // Sugar
    constexpr Recipes_Type sugar = {
        .item = nullptr,
        .tile = nullptr,
        .instance = {
            .count = 1,
            .id = 353,
            .auxiliary = 0
        },
        .letter = 's'
    };
    // Wheat
    constexpr Recipes_Type wheat = {
        .item = nullptr,
        .tile = nullptr,
        .instance = {
            .count = 1,
            .id = 296,
            .auxiliary = 0
        },
        .letter = 'w'
    };
    // Eggs
    constexpr Recipes_Type eggs = {
        .item = nullptr,
        .tile = nullptr,
        .instance = {
            .count = 1,
            .id = 344,
            .auxiliary = 0
        },
        .letter = 'e'
    };
    // Milk
    constexpr Recipes_Type milk = {
        .item = nullptr,
        .tile = nullptr,
        .instance = {
            .count = 1,
            .id = 325,
            .auxiliary = 1
        },
        .letter = 'm'
    };
    // Cake
    ItemInstance cake_item_obj = {
        .count = 1,
        .id = cake_item->id,
        .auxiliary = 0
    };
    // Add
    const std::string line1 = "mmm";
    const std::string line2 = "ses";
    const std::string line3 = "www";
    const std::vector ingredients = {milk, sugar, wheat, eggs};
    recipes->addShapedRecipe_3(cake_item_obj, line1, line2, line3, ingredients);
}

// Init
void init_cake() {
    // Add Cakes
    if (feature_has("Add Cake", server_enabled)) {
        misc_run_on_tiles_setup(Tile_initTiles_injection);
        misc_run_on_items_setup(Item_initTiles_injection);
        misc_run_on_creative_inventory_setup(Inventory_setupDefault_FillingContainer_addItem_injection);
        if (buckets_enabled) {
            // The recipe needs milk buckets
            misc_run_on_recipes_setup(Recipes_injection);
        }
    }
}
