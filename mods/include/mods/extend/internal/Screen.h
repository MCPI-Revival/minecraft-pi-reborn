#pragma once

// Custom Screen
CREATE_HELPER(Screen)
    // Functions
    virtual void init();
    virtual void render(int x, int y, float param_1);
    virtual void setupPositions();
    virtual bool handleBackEvent(bool do_nothing);
    virtual void tick();
    virtual void buttonClicked(Button *button);
    virtual void mouseClicked(int x, int y, int param_1);
    virtual void mouseReleased(int x, int y, int param_1);
    virtual void keyPressed(int key);
    virtual void keyboardNewChar(char key);
};