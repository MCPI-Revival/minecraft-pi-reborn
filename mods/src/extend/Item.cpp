#include "internal.h"

// Easily Create Custom Items
ItemInstance *CustomItem::getCraftingRemainingItem(ItemInstance *item_instance) {
    return Item_vtable::base->getCraftingRemainingItem(self, item_instance);
}
std::string CustomItem::getDescriptionId(const ItemInstance *item_instance) {
    return Item_vtable::base->getDescriptionId(self, item_instance);
}
int CustomItem::getIcon(const int auxiliary) {
    return Item_vtable::base->getIcon(self, auxiliary);
}
int CustomItem::getUseAnimation() {
    return Item_vtable::base->getUseAnimation(self);
}
int CustomItem::getUseDuration(ItemInstance *item_instance) {
    return Item_vtable::base->getUseDuration(self, item_instance);
}
bool CustomItem::isFood() {
    return Item_vtable::base->isFood(self);
}
ItemInstance *CustomItem::use(ItemInstance *item_instance, Level *level, Player *player) {
    return Item_vtable::base->use(self, item_instance, level, player);
}
int CustomItem::useOn(ItemInstance *item_instance, Player *player, Level *level, const int x, const int y, const int z, const int hit_side, const float hit_x, const float hit_y, const float hit_z) {
    return Item_vtable::base->useOn(self, item_instance, player, level, x, y, z, hit_side, hit_x, hit_y, hit_z);
}
ItemInstance CustomItem::useTimeDepleted(ItemInstance *item_instance, Level *level, Player *player) {
    return Item_vtable::base->useTimeDepleted(self, item_instance, level, player);
}

// VTable
SETUP_VTABLE(Item)
    PATCH_VTABLE(getCraftingRemainingItem);
    PATCH_VTABLE(getDescriptionId);
    PATCH_VTABLE(getIcon);
    PATCH_VTABLE(getUseAnimation);
    PATCH_VTABLE(getUseDuration);
    PATCH_VTABLE(isFood);
    PATCH_VTABLE(useOn);
    PATCH_VTABLE(use);
    PATCH_VTABLE(useTimeDepleted);
}