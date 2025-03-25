#include "internal.h"

// Easily Create Custom Tiles
AABB *CustomTile::getAABB(Level *level, const int x, const int y, const int z) {
    return _VTable::base->getAABB(self, level, x, y, z);
}
int CustomTile::getColor(LevelSource *level_source, const int x, const int y, const int z) {
    return _VTable::base->getColor(self, level_source, x, y, z);
}
std::string CustomTile::getDescriptionId() {
    return _VTable::base->getDescriptionId(self);
}
int CustomTile::getRenderLayer() {
    return _VTable::base->getRenderLayer(self);
}
int CustomTile::getTexture2(const int face, const int data) {
    return _VTable::base->getTexture2(self, face, data);
}
int CustomTile::getTexture3(LevelSource *level, const int x, const int y, const int z, const int face) {
    return _VTable::base->getTexture3(self, level, x, y, z, face);
}
bool CustomTile::isCubeShaped() {
    return _VTable::base->isCubeShaped(self);
}
bool CustomTile::isSolidRender() {
    return _VTable::base->isSolidRender(self);
}
bool CustomTile::shouldRenderFace(LevelSource *level_source, const int x, const int y, const int z, const int face) {
    return _VTable::base->shouldRenderFace(self, level_source, x, y, z, face);
}
void CustomTile::updateDefaultShape() {
    _VTable::base->updateDefaultShape(self);
}
void CustomTile::updateShape(LevelSource *level, const int x, const int y, const int z) {
    _VTable::base->updateShape(self, level, x, y, z);
}
int CustomTile::use(Level *level, const int x, const int y, const int z, Player *player) {
    return _VTable::base->use(self, level, x, y, z, player);
}

// VTable
SETUP_VTABLE(Tile)
    PATCH_VTABLE(getAABB);
    PATCH_VTABLE(getColor);
    PATCH_VTABLE(getDescriptionId);
    PATCH_VTABLE(getRenderLayer);
    PATCH_VTABLE(getTexture2);
    PATCH_VTABLE(getTexture3);
    PATCH_VTABLE(isCubeShaped);
    PATCH_VTABLE(isSolidRender);
    PATCH_VTABLE(shouldRenderFace);
    PATCH_VTABLE(updateDefaultShape);
    PATCH_VTABLE(updateShape);
    PATCH_VTABLE(use);
}