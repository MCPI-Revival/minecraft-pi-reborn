#include <libreborn/libreborn.h>
#include <symbols/minecraft.h>

#include <mods/feature/feature.h>
#include <mods/init/init.h>
#include <mods/misc/misc.h>
#include <mods/bucket/bucket.h>

static Tile *cake = NULL;

#define CAKE_LEN 0.0625F

// Description
static std::string Cake_getDescriptionId(__attribute__((unused)) Tile *tile) {
    return "tile.cake";
}

// Textures
static int Cake_getTexture2(__attribute__((unused)) Tile *tile, int face, __attribute__((unused)) int data) {
    if (face == 1) {
        // Top texture
        return 121;
    } else if (face == 0) {
        // Bottom texture
        return 124;
    }
    // Side texture
    return 122;
}

static int Cake_getTexture3(__attribute__((unused)) Tile *tile, LevelSource *level, int x, int y, int z, int face) {
    // Eaten face
    if (face == 3) {
        int data = level->vtable->getData(level, x, y, z);
        if (data != 0 && data < 6) {
            // Sliced texture
            return 123;
        }
    }
    // Normal
    return Cake_getTexture2(tile, face, 0);
}

// Rendering
static bool Cake_isSolidRender(__attribute__((unused)) Tile *tile) {
    // Stop it from turning other blocks invisable
    return 0;
}

static int Cake_getRenderLayer(__attribute__((unused)) Tile *tile) {
    // Stop weird transparency issues
    return 1;
}

static bool Cake_isCubeShaped(__attribute__((unused)) Tile *tile) {
    return false;
}

// Size
static void Cake_updateDefaultShape(Tile *tile) {
    // Set the default shape
    tile->vtable->setShape(
        tile,
        CAKE_LEN,       0.0, CAKE_LEN,
        1.0 - CAKE_LEN, 0.5, 1.0 - CAKE_LEN
    );
}

static AABB *Cake_getAABB(Tile *tile, Level *level, int x, int y, int z) {
    // Get the size of the slices
    int data = level->vtable->getData(level, x, y, z);
    if (data >= 6) data = 0;
    float slice_size = (1.0 / 7.0) * (float) data;

    // Corner 1
    AABB *aabb = &tile->aabb;
    aabb->x1 = (float) x + CAKE_LEN;
    aabb->y1 = (float) y;
    aabb->z1 = (float) z + CAKE_LEN;

    // Corner 2
    aabb->x2 = (float) x + (1.0 - CAKE_LEN);
    aabb->y2 = (float) y + 0.5;
    aabb->z2 = (float) z + (1.0 - CAKE_LEN) - slice_size;

    return aabb;
}

static void Cake_updateShape(Tile *tile, LevelSource *level, int x, int y, int z) {
    // Set cake
    int data = level->vtable->getData(level, x, y, z);
    if (data >= 6) data = 0;
    // Get slice amount
    float slice_size = (1.0 / 7.0) * (float) data;
    tile->vtable->setShape(
        tile,
        CAKE_LEN,       0.0, CAKE_LEN,
        1.0 - CAKE_LEN, 0.5, (1.0 - CAKE_LEN) - slice_size
    );
}

// Eating
static int Cake_use(__attribute__((unused)) Tile *tile, Level *level, int x, int y, int z, Player *player) {
    // Eat
    SimpleFoodData_eat(&player->foodData, 3);
    // Set the new tile
    int data = level->vtable->getData(level, x, y, z);
    if (data >= 5) {
        // Remove the cake, it has been completely gobbled up
        Level_setTileAndData(level, x, y, z, 0, 0);
    } else {
        // Remove a slice
        Level_setTileAndData(level, x, y, z, 92, data + 1);
    }
    return 1;
}

// Makes the cakes
static void make_cake() {
    // Construct
    cake = alloc_Tile();
    ALLOC_CHECK(cake);
    int texture = 122;
    Tile_constructor(cake, 92, texture, Material_dirt);
    cake->texture = texture;

    // Set VTable
    cake->vtable = dup_Tile_vtable(Tile_vtable_base);
    ALLOC_CHECK(cake->vtable);

    // Set shape
    cake->vtable->setShape(
        cake,
        CAKE_LEN,       0.0, CAKE_LEN,
        1.0 - CAKE_LEN, 0.5, 1.0 - CAKE_LEN
    );

    // Modify functions
    cake->vtable->getDescriptionId = Cake_getDescriptionId;
    cake->vtable->getTexture3 = Cake_getTexture3;
    cake->vtable->getTexture2 = Cake_getTexture2;
    cake->vtable->isSolidRender = Cake_isSolidRender;
    cake->vtable->getRenderLayer = Cake_getRenderLayer;
    cake->vtable->isCubeShaped = Cake_isCubeShaped;
    cake->vtable->updateShape = Cake_updateShape;
    cake->vtable->updateDefaultShape = Cake_updateDefaultShape;
    cake->vtable->getAABB = Cake_getAABB;
    cake->vtable->use = Cake_use;

    // Init
    Tile_init(cake);
    cake->vtable->setDestroyTime(cake, 1.0f);
    cake->vtable->setExplodeable(cake, 20.0f);
    cake->category = 4;
    std::string name = "Cake";
    cake->vtable->setDescriptionId(cake, &name);
}

static void Tile_initTiles_injection(__attribute__((unused)) void *null) {
    make_cake();
}

// Add cake to creative inventory
static void Inventory_setupDefault_FillingContainer_addItem_call_injection(FillingContainer *filling_container) {
    ItemInstance *cake_instance = new ItemInstance;
    ALLOC_CHECK(cake_instance);
    cake_instance->count = 255;
    cake_instance->auxiliary = 0;
    cake_instance->id = 92;
    FillingContainer_addItem(filling_container, cake_instance);
}

// Recipe (only when buckets are enabled)
static void Recipes_injection(Recipes *recipes) {
    // Sugar
    Recipes_Type sugar = {
        .item = 0,
        .tile = 0,
        .instance = {
            .count = 1,
            .id = 353,
            .auxiliary = 0
        },
        .letter = 's'
    };
    // Wheat
    Recipes_Type wheat = {
        .item = 0,
        .tile = 0,
        .instance = {
            .count = 1,
            .id = 296,
            .auxiliary = 0
        },
        .letter = 'w'
    };
    // Eggs
    Recipes_Type eggs = {
        .item = 0,
        .tile = 0,
        .instance = {
            .count = 1,
            .id = 344,
            .auxiliary = 0
        },
        .letter = 'e'
    };
    // Milk
    Recipes_Type milk = {
        .item = 0,
        .tile = 0,
        .instance = {
            .count = 1,
            .id = 325,
            .auxiliary = 1
        },
        .letter = 'm'
    };
    // Cake
    ItemInstance cake_item = {
        .count = 1,
        .id = 92,
        .auxiliary = 0
    };
    // Add
    std::string line1 = "mmm";
    std::string line2 = "ses";
    std::string line3 = "www";
    std::vector<Recipes_Type> ingredients = {milk, sugar, wheat, eggs};
    Recipes_addShapedRecipe_3(recipes, &cake_item, &line1, &line2, &line3, &ingredients);
}

void init_cake() {
    // Add cakes
    if (feature_has("Add Cake", server_enabled)) {
        misc_run_on_tiles_setup(Tile_initTiles_injection);
        misc_run_on_creative_inventory_setup(Inventory_setupDefault_FillingContainer_addItem_call_injection);
        if (buckets_enabled) {
            // The recipe needs milk buckets
            misc_run_on_recipes_setup(Recipes_injection);
        }
    }
}
