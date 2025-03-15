#include "internal.h"

// Easily Create Custom Tiles
AABB *CustomTile::getAABB(Level *level, const int x, const int y, const int z) {
    return Tile_vtable::base->getAABB(self, level, x, y, z);
}
int CustomTile::getColor(LevelSource *level_source, const int x, const int y, const int z) {
    return Tile_vtable::base->getColor(self, level_source, x, y, z);
}
std::string CustomTile::getDescriptionId() {
    return Tile_vtable::base->getDescriptionId(self);
}
int CustomTile::getRenderLayer() {
    return Tile_vtable::base->getRenderLayer(self);
}
int CustomTile::getTexture2(const int face, const int data) {
    return Tile_vtable::base->getTexture2(self, face, data);
}
int CustomTile::getTexture3(LevelSource *level, const int x, const int y, const int z, const int face) {
    return Tile_vtable::base->getTexture3(self, level, x, y, z, face);
}
bool CustomTile::isCubeShaped() {
    return Tile_vtable::base->isCubeShaped(self);
}
bool CustomTile::isSolidRender() {
    return Tile_vtable::base->isSolidRender(self);
}
bool CustomTile::shouldRenderFace(LevelSource *level_source, const int x, const int y, const int z, const int face) {
    return Tile_vtable::base->shouldRenderFace(self, level_source, x, y, z, face);
}
void CustomTile::updateDefaultShape() {
    Tile_vtable::base->updateDefaultShape(self);
}
void CustomTile::updateShape(LevelSource *level, const int x, const int y, const int z) {
    Tile_vtable::base->updateShape(self, level, x, y, z);
}
int CustomTile::use(Level *level, const int x, const int y, const int z, Player *player) {
    return Tile_vtable::base->use(self, level, x, y, z, player);
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