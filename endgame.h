#ifndef ENDGAME_H
    #define ENDGAME_H

#include "FPoint.h"

class BaseTexture;
class BaseButton;
class LocalEffect;

//class SparkExplosion;

class EndGame
{
    enum ENDGAME_MODE
    {
        ENDGAME_VICTORY_INFO=0,
        ENDGAME_LOSS_INFO,
        ENDGAME_TIMED_SCREEN
    };

    enum ENDGAME_BUTTONS
    {
        ID_BTN_CONTINUE=0,
        ID_BTN_SELECT,
        NUM_ENDGAME_BUTTONS,
        NUM_MESSAGES=8
    };

    BaseTexture* texButtons;
    BaseTexture* texBackground;
    BaseTexture* texTotalMessages;

    BaseTexture* texUnlocked;
    LocalEffect* unlockedEffect;
    FPoint unlockedPos;

    BaseTexture* msg[NUM_MESSAGES];

    BaseTexture* texStars[3];

    SDL_Rect rc;
    SDL_Rect rcUnlocked;

    BaseButton* buttons[NUM_ENDGAME_BUTTONS];

    bool showStar[3];
    int starMessageIndex[3];

    int mode;
    float timePassed;
    bool done;
    bool playLevel;
    bool pressedContinue;
    int unlocked;

    bool hasWon;
    int lastLevel;
    bool newScore,newTime,newShots;

    void initButtons();
    void loadImages();
    void createMessages();
    void createStaticMessageImage();
    void createUnlockedEffect();
    void renderMessages(const FPoint& offset=FPoint(0.f,0.f));

public:

    EndGame(const SDL_Rect& rc, int lastLevel, bool hasWon, bool scoreRecord, bool timeRecord, bool shotRecord, int unlockedFeature);
    ~EndGame();

    bool isDone( bool *playOn );
    bool hitContinue();

    void logic(float dt);
    void render(float phase =0.f);
    void handleEvents(SDL_Event& event);
};


#endif // ENDGAME_H
