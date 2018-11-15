
#ifndef ABOUTSCREEN_H
#define ABOUTSCREEN_H

#include "FPoint.h"

class TextureManager;
class BaseButton;
class BaseTexture;

class AboutScreen: public GameState
{
    enum ABOUT_HANDLES
    {
        HANDLE_LOGO=0,
        HANDLE_BUTTONS,
        NUM_HANDLES,
        NUM_MESSAGES=35
    };

    int handles[NUM_HANDLES];

    BaseTexture* texMessages[NUM_MESSAGES];

    BaseButton* closeButton;

    SDL_Rect scrollRect;

    float velocity;
    float leadpoint;

    FPoint positions[NUM_MESSAGES];

    FPoint dimensions[NUM_MESSAGES];

    int gaps[NUM_MESSAGES];

    int alpha[NUM_MESSAGES];

    void initButtons();
    void unloadImages();
    void loadImages();
    void createMessages();

    void initMovement();
    void moveObjects(float dt);
    void setAlphaValues();
    void setGaps();

  public:

    AboutScreen();
    ~AboutScreen();

    void Logic(float dt, float phase=0.);
    void Render(float phase=0.);
    void HandleEvents();
};

#endif // ABOUT_H
