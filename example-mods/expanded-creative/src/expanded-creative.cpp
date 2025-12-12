#include <libreborn/log.h>

#include <symbols/FillingContainer.h>
#include <symbols/ItemInstance.h>

#include <mods/misc/misc.h>

// The Actual Mod
static void Inventory_setupDefault_FillingContainer_addItem_injection(FillingContainer *filling_container) {
    ItemInstance *fire_instance = new ItemInstance;
    fire_instance->count = 255;
    fire_instance->auxiliary = 0;
    fire_instance->id = 51;
    filling_container->addItem(fire_instance);

    ItemInstance *mushroomStew_instance = new ItemInstance;
    mushroomStew_instance->count = 255;
    mushroomStew_instance->auxiliary = 0;
    mushroomStew_instance->id = 282;
    filling_container->addItem(mushroomStew_instance);

    ItemInstance *steak_instance = new ItemInstance;
    steak_instance->count = 255;
    steak_instance->auxiliary = 0;
    steak_instance->id = 364;
    filling_container->addItem(steak_instance);

    ItemInstance *cookedChicken_instance = new ItemInstance;
    cookedChicken_instance->count = 255;
    cookedChicken_instance->auxiliary = 0;
    cookedChicken_instance->id = 366;
    filling_container->addItem(cookedChicken_instance);

    ItemInstance *porkCooked_instance = new ItemInstance;
    porkCooked_instance->count = 255;
    porkCooked_instance->auxiliary = 0;
    porkCooked_instance->id = 320;
    filling_container->addItem(porkCooked_instance);

    ItemInstance *apple_instance = new ItemInstance;
    apple_instance->count = 255;
    apple_instance->auxiliary = 0;
    apple_instance->id = 260;
    filling_container->addItem(apple_instance);

    ItemInstance *tallGrass_instance = new ItemInstance;
    tallGrass_instance->count = 255;
    tallGrass_instance->auxiliary = 0;
    tallGrass_instance->id = 31;
    filling_container->addItem(tallGrass_instance);

    ItemInstance *crops_instance = new ItemInstance;
    crops_instance->count = 255;
    crops_instance->auxiliary = 0;
    crops_instance->id = 59;
    filling_container->addItem(crops_instance);

    ItemInstance *farmland_instance = new ItemInstance;
    farmland_instance->count = 255;
    farmland_instance->auxiliary = 0;
    farmland_instance->id = 60;
    filling_container->addItem(farmland_instance);

    ItemInstance *activeFurnace_instance = new ItemInstance;
    activeFurnace_instance->count = 255;
    activeFurnace_instance->auxiliary = 0;
    activeFurnace_instance->id = 62;
    filling_container->addItem(activeFurnace_instance);

    ItemInstance *ironDoor_instance = new ItemInstance;
    ironDoor_instance->count = 255;
    ironDoor_instance->auxiliary = 0;
    ironDoor_instance->id = 330;
    filling_container->addItem(ironDoor_instance);

    ItemInstance *activeRedstoneOre_instance = new ItemInstance;
    activeRedstoneOre_instance->count = 255;
    activeRedstoneOre_instance->auxiliary = 0;
    activeRedstoneOre_instance->id = 74;
    filling_container->addItem(activeRedstoneOre_instance);

    ItemInstance *pumkinStem_instance = new ItemInstance;
    pumkinStem_instance->count = 255;
    pumkinStem_instance->auxiliary = 0;
    pumkinStem_instance->id = 105;
    filling_container->addItem(pumkinStem_instance);

    ItemInstance *newGrass_instance = new ItemInstance;
    newGrass_instance->count = 255;
    newGrass_instance->auxiliary = 0;
    newGrass_instance->id = 253;
    filling_container->addItem(newGrass_instance);

    ItemInstance *reserved6_instance = new ItemInstance;
    reserved6_instance->count = 255;
    reserved6_instance->auxiliary = 0;
    reserved6_instance->id = 1;
    filling_container->addItem(reserved6_instance);

    ItemInstance *doubleStoneSlab_instance = new ItemInstance;
    doubleStoneSlab_instance->count = 255;
    doubleStoneSlab_instance->auxiliary = 0;
    doubleStoneSlab_instance->id = 43;
    filling_container->addItem(doubleStoneSlab_instance);

    ItemInstance *arrow_instance = new ItemInstance;
    arrow_instance->count = 255;
    arrow_instance->auxiliary = 0;
    arrow_instance->id = 262;
    filling_container->addItem(arrow_instance);

    ItemInstance *coal_instance = new ItemInstance;
    coal_instance->count = 255;
    coal_instance->auxiliary = 0;
    coal_instance->id = 263;
    filling_container->addItem(coal_instance);

    ItemInstance *diamond_instance = new ItemInstance;
    diamond_instance->count = 255;
    diamond_instance->auxiliary = 0;
    diamond_instance->id = 264;
    filling_container->addItem(diamond_instance);

    ItemInstance *ironIngot_instance = new ItemInstance;
    ironIngot_instance->count = 255;
    ironIngot_instance->auxiliary = 0;
    ironIngot_instance->id = 265;
    filling_container->addItem(ironIngot_instance);

    ItemInstance *goldIngot_instance = new ItemInstance;
    goldIngot_instance->count = 255;
    goldIngot_instance->auxiliary = 0;
    goldIngot_instance->id = 266;
    filling_container->addItem(goldIngot_instance);

    ItemInstance *woodSword_instance = new ItemInstance;
    woodSword_instance->count = 255;
    woodSword_instance->auxiliary = 0;
    woodSword_instance->id = 268;
    filling_container->addItem(woodSword_instance);

    ItemInstance *woodShovel_instance = new ItemInstance;
    woodShovel_instance->count = 255;
    woodShovel_instance->auxiliary = 0;
    woodShovel_instance->id = 269;
    filling_container->addItem(woodShovel_instance);

    ItemInstance *woodPickaxe_instance = new ItemInstance;
    woodPickaxe_instance->count = 255;
    woodPickaxe_instance->auxiliary = 0;
    woodPickaxe_instance->id = 270;
    filling_container->addItem(woodPickaxe_instance);

    ItemInstance *woodAxe_instance = new ItemInstance;
    woodAxe_instance->count = 255;
    woodAxe_instance->auxiliary = 0;
    woodAxe_instance->id = 271;
    filling_container->addItem(woodAxe_instance);

    ItemInstance *stoneSword_instance = new ItemInstance;
    stoneSword_instance->count = 255;
    stoneSword_instance->auxiliary = 0;
    stoneSword_instance->id = 272;
    filling_container->addItem(stoneSword_instance);

    ItemInstance *stoneShovel_instance = new ItemInstance;
    stoneShovel_instance->count = 255;
    stoneShovel_instance->auxiliary = 0;
    stoneShovel_instance->id = 273;
    filling_container->addItem(stoneShovel_instance);

    ItemInstance *stonePickaxe_instance = new ItemInstance;
    stonePickaxe_instance->count = 255;
    stonePickaxe_instance->auxiliary = 0;
    stonePickaxe_instance->id = 274;
    filling_container->addItem(stonePickaxe_instance);

    ItemInstance *stoneAxe_instance = new ItemInstance;
    stoneAxe_instance->count = 255;
    stoneAxe_instance->auxiliary = 0;
    stoneAxe_instance->id = 275;
    filling_container->addItem(stoneAxe_instance);

    ItemInstance *shovelIron_instance = new ItemInstance;
    shovelIron_instance->count = 255;
    shovelIron_instance->auxiliary = 0;
    shovelIron_instance->id = 256;
    filling_container->addItem(shovelIron_instance);

    ItemInstance *ironPick_instance = new ItemInstance;
    ironPick_instance->count = 255;
    ironPick_instance->auxiliary = 0;
    ironPick_instance->id = 257;
    filling_container->addItem(ironPick_instance);

    ItemInstance *ironAxe_instance = new ItemInstance;
    ironAxe_instance->count = 255;
    ironAxe_instance->auxiliary = 0;
    ironAxe_instance->id = 258;
    filling_container->addItem(ironAxe_instance);

    ItemInstance *diamondSword_instance = new ItemInstance;
    diamondSword_instance->count = 255;
    diamondSword_instance->auxiliary = 0;
    diamondSword_instance->id = 276;
    filling_container->addItem(diamondSword_instance);

    ItemInstance *diamondShovel_instance = new ItemInstance;
    diamondShovel_instance->count = 255;
    diamondShovel_instance->auxiliary = 0;
    diamondShovel_instance->id = 277;
    filling_container->addItem(diamondShovel_instance);

    ItemInstance *diamondPickaxe_instance = new ItemInstance;
    diamondPickaxe_instance->count = 255;
    diamondPickaxe_instance->auxiliary = 0;
    diamondPickaxe_instance->id = 278;
    filling_container->addItem(diamondPickaxe_instance);

    ItemInstance *diamondAxe_instance = new ItemInstance;
    diamondAxe_instance->count = 255;
    diamondAxe_instance->auxiliary = 0;
    diamondAxe_instance->id = 279;
    filling_container->addItem(diamondAxe_instance);

    ItemInstance *magicWand_instance = new ItemInstance;
    magicWand_instance->count = 255;
    magicWand_instance->auxiliary = 0;
    magicWand_instance->id = 280;
    filling_container->addItem(magicWand_instance);

    ItemInstance *bowl_instance = new ItemInstance;
    bowl_instance->count = 255;
    bowl_instance->auxiliary = 0;
    bowl_instance->id = 281;
    filling_container->addItem(bowl_instance);

    ItemInstance *goldSword_instance = new ItemInstance;
    goldSword_instance->count = 255;
    goldSword_instance->auxiliary = 0;
    goldSword_instance->id = 283;
    filling_container->addItem(goldSword_instance);

    ItemInstance *goldShovel_instance = new ItemInstance;
    goldShovel_instance->count = 255;
    goldShovel_instance->auxiliary = 0;
    goldShovel_instance->id = 284;
    filling_container->addItem(goldShovel_instance);

    ItemInstance *goldPickaxe_instance = new ItemInstance;
    goldPickaxe_instance->count = 255;
    goldPickaxe_instance->auxiliary = 0;
    goldPickaxe_instance->id = 285;
    filling_container->addItem(goldPickaxe_instance);

    ItemInstance *goldAxe_instance = new ItemInstance;
    goldAxe_instance->count = 255;
    goldAxe_instance->auxiliary = 0;
    goldAxe_instance->id = 286;
    filling_container->addItem(goldAxe_instance);

    ItemInstance *string_instance = new ItemInstance;
    string_instance->count = 255;
    string_instance->auxiliary = 0;
    string_instance->id = 287;
    filling_container->addItem(string_instance);

    ItemInstance *feather_instance = new ItemInstance;
    feather_instance->count = 255;
    feather_instance->auxiliary = 0;
    feather_instance->id = 288;
    filling_container->addItem(feather_instance);

    ItemInstance *gunpowder_instance = new ItemInstance;
    gunpowder_instance->count = 255;
    gunpowder_instance->auxiliary = 0;
    gunpowder_instance->id = 289;
    filling_container->addItem(gunpowder_instance);

    ItemInstance *woodHoe_instance = new ItemInstance;
    woodHoe_instance->count = 255;
    woodHoe_instance->auxiliary = 0;
    woodHoe_instance->id = 290;
    filling_container->addItem(woodHoe_instance);

    ItemInstance *stoneHoe_instance = new ItemInstance;
    stoneHoe_instance->count = 255;
    stoneHoe_instance->auxiliary = 0;
    stoneHoe_instance->id = 291;
    filling_container->addItem(stoneHoe_instance);

    ItemInstance *flint1_instance = new ItemInstance;
    flint1_instance->count = 255;
    flint1_instance->auxiliary = 0;
    flint1_instance->id = 292;
    filling_container->addItem(flint1_instance);

    ItemInstance *diamondHoe_instance = new ItemInstance;
    diamondHoe_instance->count = 255;
    diamondHoe_instance->auxiliary = 0;
    diamondHoe_instance->id = 293;
    filling_container->addItem(diamondHoe_instance);

    ItemInstance *goldHoe_instance = new ItemInstance;
    goldHoe_instance->count = 255;
    goldHoe_instance->auxiliary = 0;
    goldHoe_instance->id = 294;
    filling_container->addItem(goldHoe_instance);

    ItemInstance *seeds_instance = new ItemInstance;
    seeds_instance->count = 255;
    seeds_instance->auxiliary = 0;
    seeds_instance->id = 295;
    filling_container->addItem(seeds_instance);

    ItemInstance *wheat_instance = new ItemInstance;
    wheat_instance->count = 255;
    wheat_instance->auxiliary = 0;
    wheat_instance->id = 296;
    filling_container->addItem(wheat_instance);

    ItemInstance *bread_instance = new ItemInstance;
    bread_instance->count = 255;
    bread_instance->auxiliary = 0;
    bread_instance->id = 297;
    filling_container->addItem(bread_instance);

    ItemInstance *diamondHelm_instance = new ItemInstance;
    diamondHelm_instance->count = 255;
    diamondHelm_instance->auxiliary = 0;
    diamondHelm_instance->id = 310;
    filling_container->addItem(diamondHelm_instance);

    ItemInstance *diamondChest_instance = new ItemInstance;
    diamondChest_instance->count = 255;
    diamondChest_instance->auxiliary = 0;
    diamondChest_instance->id = 311;
    filling_container->addItem(diamondChest_instance);

    ItemInstance *diamondLeg_instance = new ItemInstance;
    diamondLeg_instance->count = 255;
    diamondLeg_instance->auxiliary = 0;
    diamondLeg_instance->id = 312;
    filling_container->addItem(diamondLeg_instance);

    ItemInstance *diamondBoot_instance = new ItemInstance;
    diamondBoot_instance->count = 255;
    diamondBoot_instance->auxiliary = 0;
    diamondBoot_instance->id = 313;
    filling_container->addItem(diamondBoot_instance);

    ItemInstance *leatherCap_instance = new ItemInstance;
    leatherCap_instance->count = 255;
    leatherCap_instance->auxiliary = 0;
    leatherCap_instance->id = 298;
    filling_container->addItem(leatherCap_instance);

    ItemInstance *leatherShirt_instance = new ItemInstance;
    leatherShirt_instance->count = 255;
    leatherShirt_instance->auxiliary = 0;
    leatherShirt_instance->id = 299;
    filling_container->addItem(leatherShirt_instance);

    ItemInstance *leatherPants_instance = new ItemInstance;
    leatherPants_instance->count = 255;
    leatherPants_instance->auxiliary = 0;
    leatherPants_instance->id = 300;
    filling_container->addItem(leatherPants_instance);

    ItemInstance *leatherBoots_instance = new ItemInstance;
    leatherBoots_instance->count = 255;
    leatherBoots_instance->auxiliary = 0;
    leatherBoots_instance->id = 301;
    filling_container->addItem(leatherBoots_instance);

    ItemInstance *chainHelm_instance = new ItemInstance;
    chainHelm_instance->count = 255;
    chainHelm_instance->auxiliary = 0;
    chainHelm_instance->id = 302;
    filling_container->addItem(chainHelm_instance);

    ItemInstance *chainShirt_instance = new ItemInstance;
    chainShirt_instance->count = 255;
    chainShirt_instance->auxiliary = 0;
    chainShirt_instance->id = 303;
    filling_container->addItem(chainShirt_instance);

    ItemInstance *chainLegs_instance = new ItemInstance;
    chainLegs_instance->count = 255;
    chainLegs_instance->auxiliary = 0;
    chainLegs_instance->id = 304;
    filling_container->addItem(chainLegs_instance);

    ItemInstance *chainBoots_instance = new ItemInstance;
    chainBoots_instance->count = 255;
    chainBoots_instance->auxiliary = 0;
    chainBoots_instance->id = 305;
    filling_container->addItem(chainBoots_instance);

    ItemInstance *goldHelm_instance = new ItemInstance;
    goldHelm_instance->count = 255;
    goldHelm_instance->auxiliary = 0;
    goldHelm_instance->id = 314;
    filling_container->addItem(goldHelm_instance);

    ItemInstance *goldChest_instance = new ItemInstance;
    goldChest_instance->count = 255;
    goldChest_instance->auxiliary = 0;
    goldChest_instance->id = 315;
    filling_container->addItem(goldChest_instance);

    ItemInstance *goldLegs_instance = new ItemInstance;
    goldLegs_instance->count = 255;
    goldLegs_instance->auxiliary = 0;
    goldLegs_instance->id = 316;
    filling_container->addItem(goldLegs_instance);

    ItemInstance *goldBoots_instance = new ItemInstance;
    goldBoots_instance->count = 255;
    goldBoots_instance->auxiliary = 0;
    goldBoots_instance->id = 317;
    filling_container->addItem(goldBoots_instance);

    ItemInstance *ironHelm_instance = new ItemInstance;
    ironHelm_instance->count = 255;
    ironHelm_instance->auxiliary = 0;
    ironHelm_instance->id = 306;
    filling_container->addItem(ironHelm_instance);

    ItemInstance *ironChest_instance = new ItemInstance;
    ironChest_instance->count = 255;
    ironChest_instance->auxiliary = 0;
    ironChest_instance->id = 307;
    filling_container->addItem(ironChest_instance);

    ItemInstance *ironLegs_instance = new ItemInstance;
    ironLegs_instance->count = 255;
    ironLegs_instance->auxiliary = 0;
    ironLegs_instance->id = 308;
    filling_container->addItem(ironLegs_instance);

    ItemInstance *ironBoots_instance = new ItemInstance;
    ironBoots_instance->count = 255;
    ironBoots_instance->auxiliary = 0;
    ironBoots_instance->id = 309;
    filling_container->addItem(ironBoots_instance);

    ItemInstance *flint2_instance = new ItemInstance;
    flint2_instance->count = 255;
    flint2_instance->auxiliary = 0;
    flint2_instance->id = 318;
    filling_container->addItem(flint2_instance);

    ItemInstance *porkRaw_instance = new ItemInstance;
    porkRaw_instance->count = 255;
    porkRaw_instance->auxiliary = 0;
    porkRaw_instance->id = 319;
    filling_container->addItem(porkRaw_instance);

    ItemInstance *leather_instance = new ItemInstance;
    leather_instance->count = 255;
    leather_instance->auxiliary = 0;
    leather_instance->id = 334;
    filling_container->addItem(leather_instance);

    ItemInstance *clayBrick_instance = new ItemInstance;
    clayBrick_instance->count = 255;
    clayBrick_instance->auxiliary = 0;
    clayBrick_instance->id = 336;
    filling_container->addItem(clayBrick_instance);

    ItemInstance *clay_instance = new ItemInstance;
    clay_instance->count = 255;
    clay_instance->auxiliary = 0;
    clay_instance->id = 337;
    filling_container->addItem(clay_instance);

    ItemInstance *notepad_instance = new ItemInstance;
    notepad_instance->count = 255;
    notepad_instance->auxiliary = 0;
    notepad_instance->id = 339;
    filling_container->addItem(notepad_instance);

    ItemInstance *book_instance = new ItemInstance;
    book_instance->count = 255;
    book_instance->auxiliary = 0;
    book_instance->id = 340;
    filling_container->addItem(book_instance);

    ItemInstance *slimeball_instance = new ItemInstance;
    slimeball_instance->count = 255;
    slimeball_instance->auxiliary = 0;
    slimeball_instance->id = 341;
    filling_container->addItem(slimeball_instance);

    ItemInstance *compass_instance = new ItemInstance;
    compass_instance->count = 255;
    compass_instance->auxiliary = 0;
    compass_instance->id = 345;
    filling_container->addItem(compass_instance);

    ItemInstance *clock_instance = new ItemInstance;
    clock_instance->count = 255;
    clock_instance->auxiliary = 0;
    clock_instance->id = 347;
    filling_container->addItem(clock_instance);

    ItemInstance *glowDust_instance = new ItemInstance;
    glowDust_instance->count = 255;
    glowDust_instance->auxiliary = 0;
    glowDust_instance->id = 348;
    filling_container->addItem(glowDust_instance);

    ItemInstance *bone_instance = new ItemInstance;
    bone_instance->count = 255;
    bone_instance->auxiliary = 0;
    bone_instance->id = 352;
    filling_container->addItem(bone_instance);

    ItemInstance *sugar_instance = new ItemInstance;
    sugar_instance->count = 255;
    sugar_instance->auxiliary = 0;
    sugar_instance->id = 353;
    filling_container->addItem(sugar_instance);

    ItemInstance *melon_instance = new ItemInstance;
    melon_instance->count = 255;
    melon_instance->auxiliary = 0;
    melon_instance->id = 360;
    filling_container->addItem(melon_instance);

    ItemInstance *beefRaw_instance = new ItemInstance;
    beefRaw_instance->count = 255;
    beefRaw_instance->auxiliary = 0;
    beefRaw_instance->id = 363;
    filling_container->addItem(beefRaw_instance);

    ItemInstance *chickenRaw_instance = new ItemInstance;
    chickenRaw_instance->count = 255;
    chickenRaw_instance->auxiliary = 0;
    chickenRaw_instance->id = 365;
    filling_container->addItem(chickenRaw_instance);
}

// Init
__attribute__((constructor)) static void init_expanded_creative() {
    INFO("Loading Expanded Creative Mod");
    misc_run_on_creative_inventory_setup(Inventory_setupDefault_FillingContainer_addItem_injection);
}
