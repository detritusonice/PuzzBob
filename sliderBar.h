
#ifndef SLIDERBAR_H
#define SLIDERBAR_H

#include "FPoint.h"
class BaseTexture;


enum SLIDERSTATES
{
    SLIDERSTATE_IDLE=0,
    SLIDERSTATE_DISABLED,
    SLIDERSTATE_PRESSED,
    NUM_SLIDERSTATES
};

class BaseSlider
{
    FPoint origin;

    SDL_Rect rcBox;
    SDL_Point touchPoint;

    SDL_Color dFgColor;
    SDL_Color dBgColor;

    SDL_Rect rcBackground;
    SDL_Rect rcForeground;
    SDL_Rect rcActive;
    SDL_Rect rcHandle;

    BaseTexture* texBackground;
    BaseTexture* texForeground;
    BaseTexture* texActive;
    BaseTexture* texHandle;

    int direction;//perpendicular to movement direction of scrollpage
    int state;
    float level;

    bool isOnScrollPage;

    bool wasModified;
    bool clickOutside;

    Uint8 prevMouseState;

    void init();

    void setOrigin(int x, int y);

    void loadImages();
    void computeRects();
    void computeActive();

public:

    BaseSlider(int x, int y,int w, int h, int scrollerDirection,float newLevel,SDL_Color disabledFgClr={128,128,128}, SDL_Color disabledBgClr={50,50,50});
    ~BaseSlider();

    void render(const FPoint& offset=FPoint(0.,0.),float alphaModifier=1.);
    bool handleEvents(FPoint parentPosition=FPoint(0.,0.));

    int getState();
    float getLevel();
    FPoint getOrigin();

    int setState(int newState);
    void setPosition(int x,int y,int w=0, int h=0);
    void setLevel(float newLevel);
    void setOnScrollPage(bool isOnScrollPage=true);
    void setDirection(int sliderDirection);

    void enable();
    void disable();
    bool isEnabled();
    bool levelChanged();
};

#endif
