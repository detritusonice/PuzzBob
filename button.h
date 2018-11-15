#ifndef BUTTON_H
#define BUTTON_H

#include "FPoint.h"
class BaseTexture;

//button prototypes

enum BUTTONSTATES
{
    BUTTONSTATE_IDLE=0,
    BUTTONSTATE_HOVER,
    BUTTONSTATE_DISABLED,
    BUTTONSTATE_PUSHEDHOVER,
    BUTTONSTATE_PUSHEDIN,
    BUTTONSTATE_PRESSED,
    NUM_BUTTONSTATES
};
enum BUTTON_TYPES
{
    BT_PRESSONLY=0, //pressin
    BT_PUSHONLY,    //radiobutton
    BT_PUSHBUTTON   //push on-off
};

//-----------------------------------------------
//|             |               |               |
//|    IDLE     |     HOVER     |   DISABLED    |
//|             |               |               |
//-----------------------------------------------
//|             |               |               |
//| PUSHEDHOVER |   PUSHEDIN    |    PRESSED    |
//|             |               |               |
//-----------------------------------------------

class BaseButton
{
protected:
    SDL_Rect rcBox;
    SDL_Point touchPoint;
    FPoint origin;

    bool wasPressed;
    bool stateChange;

    bool isScrollPageButton;

    bool pushedIn;
    bool clickOutside;

    int iButtonType;
    int iButtonState;
    float stateAlphaMod;

    Uint8 prevMouseState;

    void init(bool isScrollButton=false);

    void setOrigin(int x, int y);


    public:
    virtual void render(const FPoint& offset=FPoint(0.,0.),float alphaModifier=1.)=0;
    virtual ~BaseButton(){}

    bool handleEvents(FPoint parentPosition=FPoint(0.,0.));

    int getState();
    int setState(int newState);
    void setPosition(int x,int y,int w=0, int h=0);

    FPoint getOrigin();

    void setOnScrollPage(bool isOnScrollPage=true);

    void enable();
    void disable();
    bool isEnabled();
    bool isPushedIn();
    bool gotPressed();
    bool changedState();

};

class TextureButton: public BaseButton
{
protected:
    BaseTexture* texButton;
    BaseTexture* texCaption;

    Uint8  mAlphaValue;

    void setClips();

public:

    void setCaption(const char*);

};

class SimpleButton : public TextureButton
{
public:

    SimpleButton(BaseTexture* buttonTexture,int x,int y,int w, int h, const char*strCaption,int buttonType=BT_PRESSONLY,Uint8 alphaValue=255);
    ~SimpleButton();
    void render(const FPoint& offset=FPoint(0.,0.),float alphaModifier=1.);
};

class GlassButton : public TextureButton
{
    BaseTexture* texBackground;

public:
    GlassButton(BaseTexture* backgroundTexture,  int x,int y,int w, int h, const char*strCaption,int buttonType=BT_PRESSONLY,Uint8 alphaValue=255);
    ~GlassButton();
    void render(const FPoint& offset=FPoint(0.,0.),float alphaModifier=1.);
};

class InGame;

class LevelButton: public TextureButton
{
    static InGame* theGame;

    BaseTexture* texBackground;
    BaseTexture* texPtr;

    BaseTexture* texStars[3];
    bool showStar[3];
    SDL_Rect rcStars[3];
    int numStars;

    int level;
    int invHandle;
    void createBackground(char* buffer);
    void freeBackground();
    void resetBeatenStats();
    void setBeatenStats();


public:
    LevelButton(BaseTexture* backgroundTexture,  int x,int y,int w, int h, const char*strCaption,int buttonType=BT_PRESSONLY,Uint8 alphaValue=255);
    ~LevelButton();
    void render(const FPoint& offset=FPoint(0.,0.),float alphaModifier=1.);

    void setFile(const char* fname,int level=-1,bool changeCaption=false);
    void setLevel(int newLevel,bool changeCaption=false);

    int getLevel();

    static void setGameObject(InGame* game);
};
#endif
