#ifndef SCREEN_EFFECTS_H
#define SCREEN_EFFECTS_H

#include "constants.h"
#include "FPoint.h"

class BaseTexture;

class ScreenEffect
{
    virtual void init()=0;
    public:
    virtual void resize(SDL_Rect *rcSize = screenRect )=0;
    virtual bool update(float dt=0, float phase=0)=0;
    virtual void render(float alphaModifier=1.f)=0;
    virtual void changeEffectAlpha(int diff)=0;
    virtual void moveEffect(const FPoint &newPos)=0;

    virtual void loadImages()=0;

    virtual ~ScreenEffect(){};
};

class AirBubbles : public ScreenEffect
{

    int handles[NUM_BALLCOLORS-NUM_SPECIALBALLS];
    SDL_Rect ballRect[NUM_BALLCOLORS-NUM_SPECIALBALLS];

    SDL_Rect effectRect;

    FPoint v[NUM_BALLCOLORS-NUM_SPECIALBALLS];
    FPoint p[NUM_BALLCOLORS-NUM_SPECIALBALLS];

    int effectAlpha;
    int numBalls;
    int dist_extra;

    void initBallRects();
    void init();

    public:


    void resize(SDL_Rect *rcSize=screenRect);
    bool update(float dt=0, float phase=0);
    void render(float alphaModifier=1.f);

    void changeEffectAlpha(int diff);
    void moveEffect(const FPoint &mousePos);

    void loadImages();

    AirBubbles(SDL_Rect *rc = screenRect);
    ~AirBubbles();

};

class Flash : public ScreenEffect
{

    int handle;

    SDL_Rect effectRect;
    float height;

    int alphaValue;
    int effectAlpha;
    float duration,timeLeft;

    FPoint pos;

    void init();

public:

    Flash(float effectDuration= 0.3f,SDL_Rect *rcSize = screenRect);
    ~Flash();

    void shine(float effectDuration= 0.3f,SDL_Rect *rcSize = screenRect,const FPoint& position=FPoint(0.f,0.f),float rad=0.);

    void resize(SDL_Rect *rcSize = screenRect);
    bool update(float dt=0.f, float phase=0.f);
    void render(float alphaModifier=1.f);

    void changeEffectAlpha(int diff);
    void moveEffect(const FPoint &newPos);
    void loadImages();
};

class NovaFlash : public ScreenEffect
{
    enum HANDLES
    {
        HANDLE_NOVA=0,
        HANDLE_FLASH,
        NUM_HANDLES
    };

    int handles[NUM_HANDLES];

    SDL_Rect rects[NUM_HANDLES];
    SDL_Rect effectRect;

    FPoint pos;

    int alphaValues[NUM_HANDLES];
    int effectAlpha;
    float duration,timeLeft;

    float angles[NUM_HANDLES];

    void initRects();
    void init();

public:

    NovaFlash(FPoint position, float effectDuration= 0.3f,SDL_Rect *rcSize = screenRect);
    ~NovaFlash();

    void shine(FPoint position, float effectDuration= 0.3f,SDL_Rect *rcSize = screenRect);

    void resize(SDL_Rect *rcSize = screenRect);
    bool update(float dt=0.f, float phase=0.f);
    void render(float alphaModifier=1.f);

    void changeEffectAlpha(int diff);
    void moveEffect(const FPoint &newPos);

    void loadImages();


};

class TrailsEffect
{
    BaseTexture* buf[2];
    int current;
    int width,height,length;
    SDL_Color bgColor;

public:
    TrailsEffect(int w, int h, const SDL_Color& clr={0,0,0}, int len=10);
    ~TrailsEffect();

    void initFrame( const SDL_Color& clr);
    void initFrame( );
    void renderPreviousFrame();
    void includeInEffect();
    void excludeFromEffect();
    void renderFrame(int x, int y);
};

#endif // SCREEN_EFFECTS_H
