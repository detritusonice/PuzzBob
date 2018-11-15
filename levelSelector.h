#ifndef LEVEL_SELECTOR_H
#define LEVEL_SELECTOR_H

#include "scrollPage.h"

class LevelPage: public ScrollPage
{
    void setPageContent( int newID);

    int *selectedLevel;
    int dim;
    int maxLevel;

    float offset;
    int buttonWidth;
    int buttonHeight;

    void initControls();
    void loadImages();
    void setButtonStates();
    int getButtonX(int id);
    int getButtonY(int id);

public:
    LevelPage(std::string titleStr, const FPoint& pos, const SDL_Rect& rect, int* levelPtr,int levelMax);
    ~LevelPage();

    void render(float alphaModifier =1.);
    bool handleEvents();
    void renderTitle(float alphaModifier=1.);
};

//=============================================

class LevelSelector
{
    ManhattanPageHandler* pageHandler;

    int action, level;
    bool done;

    void populatePages(int maxLevel);

public:
    LevelSelector( int startinglevel);
    ~LevelSelector();

    void handleEvents(SDL_Event& event);
    void render(float phase=0.);
    void logic(float dt, float phase=0.);

    bool isDone(int *actn, int *levl);
};

#endif // LEVEL_SELECTOR_H
