
#ifndef PAUSESCREEN_H
    #define PAUSESCREEN_H
#include "FPoint.h"

class BaseTexture;
class BaseButton;
//class SparkExplosion;

class PauseScreen
{
    enum PAUSE_MODE
    {
        MODE_PAUSE=0,
        MODE_QUIT
    };

    enum PAUSE_BUTTONS
    {
        ID_BTN_1=0,
        ID_BTN_2,
        NUM_PAUSE_BUTTONS,
        NUM_MESSAGES=12
    };

    BaseTexture* texButtons[2];
    BaseTexture* texBackground;
    BaseTexture* texTotalMessages;

    BaseTexture* msg[NUM_MESSAGES];

    SDL_Rect rc;

    BaseButton* buttons[NUM_PAUSE_BUTTONS];

    bool isPauseScreen;
    bool isGameInProgress;

    bool done;

    bool quitGame;

    int level;
    int duration, score, shots;

    void initButtons();
    void loadImages();
    void createMessages();
    void createStaticMessageImage();
    void renderMessages(const FPoint& offset=FPoint(0.f,0.f));

public:

    PauseScreen(const SDL_Rect& rc, int playingLevel, bool isQuitScreen, bool gameInProgress,int roundTime, int roundShots, int roundScore, bool makeAggregates=true);
    ~PauseScreen();

    bool isDone( bool *playOn );

    void render(float phase =0.f);
    void handleEvents(SDL_Event& event);
};


#endif // ENDGAME_H
