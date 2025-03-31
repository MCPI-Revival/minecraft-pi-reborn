#pragma once

// Custom Tile
CREATE_HELPER(Tile)
    // Functions
    virtual std::string getDescriptionId();
    virtual int getTexture3(LevelSource *level, int x, int y, int z, int face);
    virtual int getTexture2(int face, int data);
    virtual bool isSolidRender();
    virtual int getRenderLayer();
    virtual bool isCubeShaped();
    virtual void updateShape(LevelSource *level, int x, int y, int z);
    virtual void updateDefaultShape();
    virtual AABB *getAABB(Level *level, int x, int y, int z);
    virtual bool use(Level *level, int x, int y, int z, Player *player);
    virtual bool shouldRenderFace(LevelSource *level_source, int x, int y, int z, int face);
    virtual int getColor(LevelSource *level_source, int x, int y, int z);
};