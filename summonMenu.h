
#ifndef SUMMONMENU_H
    #define SUMMONMENU_H

class BaseTexture;
class BaseButton;
class LocalEffect;

class FPoint;

class SummonMenu
{
    enum CONTROL_BUTTONS
    {
        ID_BTN_CANCEL=0,
        ID_BTN_DONE,
        NUM_CONTROL_BUTTONS
    };

    enum BALL_BUTTONS
    {
        ID_BTN_BOMB=0,
        ID_BTN_SHIFTER,
        ID_BTN_TIMESTOP,
        ID_BTN_PAINTER,
        ID_BTN_LEVELER,
        ID_BTN_CHROME,
        NUM_BALL_BUTTONS
    };


    BaseTexture* texButtons;
    BaseTexture* texStatic;

    BaseTexture* texMessages[2];
    BaseTexture* texBallMessages[NUM_BALL_BUTTONS+1];
    BaseTexture* texBalls[NUM_BALL_BUTTONS];
    LocalEffect* ballEffects[NUM_BALL_BUTTONS];

    SDL_Rect rc;
    SDL_Rect rcBalls[NUM_BALL_BUTTONS];

    char ballTexts[NUM_BALL_BUTTONS][30];

    int ballAlpha[NUM_BALL_BUTTONS];

    BaseButton* controlButtons[NUM_CONTROL_BUTTONS];
    BaseButton* ballButtons[NUM_BALL_BUTTONS];

    bool done;
    int level;
    bool invStatic;
    bool itemsAvailable;

    bool powerBallsAvailable;
    bool timeStopAvailable;
    bool ballAlreadySummoned;

    bool commitChanges;
    int selectedItem;

    void initButtons();
    void loadImages();
    void createMessages();
    void updateStaticImage();
    void createEffects();
    void renderMessages(const FPoint& offset);
    void renderContent(const FPoint& offset);
    bool createButtonMessage(int buttonID, int blocklevel);

public:

    SummonMenu(const SDL_Rect& rc, int playingLevel, bool timeStopUsed, bool ballSummoned, int ballChosen, bool powerballGenerated);
    ~SummonMenu();

    bool isDone( bool *commit, int* item );

    void logic(float dt);
    void render(float phase =0.f);
    void handleEvents(SDL_Event& event);
};


#endif // ENDGAME_H

