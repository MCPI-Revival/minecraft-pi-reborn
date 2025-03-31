#pragma once

// Custom Item
CREATE_HELPER(Item)
    // Functions
    virtual std::string getDescriptionId(const ItemInstance *item_instance);
    virtual int getIcon(int auxiliary);
    virtual bool useOn(ItemInstance *item_instance, Player *player, Level *level, int x, int y, int z, int hit_side, float hit_x, float hit_y, float hit_z);
    virtual int getUseDuration(ItemInstance *item_instance);
    virtual ItemInstance useTimeDepleted(ItemInstance *item_instance, Level *level, Player *player);
    virtual int getUseAnimation();
    virtual bool isFood();
    virtual ItemInstance *use(ItemInstance *item_instance, Level *level, Player *player);
    virtual ItemInstance *getCraftingRemainingItem(ItemInstance *item_instance);
};