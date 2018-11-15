#ifndef OPTIONS_MENU_H
#define OPTIONS_MENU_H

#include <string>

class TextureManager;
class SimpleButton;
class BaseTexture;
class BaseSlider;

#include "scrollPage.h"

class OptionsMenu: public GameState
{
    enum MENU_BUTTONS
    {
        ID_BTN_CANCEL,
        ID_BTN_OK,
        NUM_MENU_BUTTONS
    };

    BaseTexture* texButtons;

    PageHandler* pageHandler;

    TextureButton* menuButtons[NUM_MENU_BUTTONS];

    int randomness;
    int pressure;
    int skin;
    int match;
    int widthID;
    bool useAid;
    bool useBomb;
    bool useChrome;
    bool useShifters;
    bool usePainters;
    bool useLevelers;

    bool renderBgEffect;
    bool useTranslucency;
    bool multiExplode;
    bool useShine;
    bool fullSceneComplexity;
    bool renderTrails;
    bool flashOnExplosions;

    int musicVolume;
    int effectsVolume;

    bool showRoundScore;
    bool showRoundTime;
    bool showRoundShots;
    bool showBest;
    bool showStatsOnPause;
    bool showBeatenStats;
    bool showSwipeHint;
    bool overrideSettingsOnContinue;

    bool changedSkin;

    void initButtons();
    void populatePages();

  public:

    OptionsMenu();
    ~OptionsMenu();

    void Logic(float dt, float phase=0.);
    void Render(float phase=0.);
    void HandleEvents();

    void saveSettings();

};
//=====================================================

class GamePlayPage: public ScrollPage
{
    enum GAMEPLAY_BUTTONS
    {
        ID_BTN_RAND_LOW=0,
        ID_BTN_RAND_MODERATE,
        ID_BTN_RAND_HIGH,
        ID_BTN_RAND_HIGHEST,
        ID_BTN_PRES_NONE,
        ID_BTN_PRES_MODERATE,
        ID_BTN_PRES_HIGH,
        ID_BTN_PRES_HIGHEST,
        ID_BTN_USE_AIM,
        ID_BTN_USE_BOMBS,
        ID_BTN_USE_CHROME,
        ID_BTN_USE_SHIFTER,
        ID_BTN_USE_PAINTER,
        ID_BTN_USE_LEVELER,
        ID_BTN_LOWER,
        ID_BTN_HIGHER,
        ID_BTN_SIZE8,
        ID_BTN_SIZE10,
        ID_BTN_SIZE12,
        ID_BTN_SIZE14,
        ID_BTN_SIZE16,
        NUM_GAMEPLAY_BUTTONS
    };
    enum MESSAGE_TYPE
    {
        MESSAGETYPE_RAND=0,
        MESSAGETYPE_PRES,
        MESSAGETYPE_MATCH
    };

    BaseTexture* texMsgBackground;

    BaseTexture* texMessageRand;
    BaseTexture* texMessagePres;
    BaseTexture* texMessageAim;
    BaseTexture* texMessageBombs;
    BaseTexture* texMessageChrome;
    BaseTexture* texMessageShifter;
    BaseTexture* texMessageLeveler;
    BaseTexture* texMessagePainter;
    BaseTexture* texMessageMatch;
    BaseTexture* texMatchNumber;

    BaseTexture* texMessageWidth;

    BaseTexture* texButtons[4];

    int *randomness;
    int *pressure;
    int *matchNumber;
    int *wellWidthID;

    bool *useAimingAid;
    bool *useBombs;
    bool *useChrome;
    bool *useShifter;
    bool *usePainter;
    bool *useLeveler;

    int getColumnOffset(int column);
    int buttonWidth;

    void initControls();
    void loadImages();
    void setButtonStates();
    void setMatchButtonStates();
    void setMatchUpButtonState();
    void setMatchDownButtonState();
    void setWellWidthButtonStates();
    void constructMessage(int type);

public:
    GamePlayPage(std::string titleStr, const FPoint& pos, const SDL_Rect& rect,int* randPtr, int* presPtr,
                                        int* match, int* widthID, bool* aid, bool *bomb, bool *chrome,bool *leveler,bool *shifter,bool *painter );
    ~GamePlayPage();

    void render(float alphaModifier =1.);
    bool handleEvents();
//    void renderTitle(float alphaModifier=1.);

};

//=====================================================

class SkinPage: public ScrollPage
{

    BaseTexture* texButtons[4];

    int *skinType;
    bool isPage2;

    int buttonWidth;
    int buttonHeight;
    int offset;

    void initControls();
    void loadImages();
    void setButtonStates();

public:
    SkinPage(std::string titleStr, const FPoint& pos, const SDL_Rect& rect, int* skinPtr, bool page2 );
    ~SkinPage();

    void render(float alphaModifier =1.);
    bool handleEvents();
   // void renderTitle(float alphaModifier=1.);

};

//=====================================================

class EffectsPage: public ScrollPage
{
    enum EFFECT_SETTINGS
    {
        SETTING_TRANSLUCENT_BGS=0,
        SETTING_RENDER_BGEFFECT,
        SETTING_SHINE_EFFECT,
        SETTING_MULTI_EXPLODE,
        SETTING_TRAILS,
        SETTING_FULL_COMPLEXITY,
        SETTING_FLASH,
        NUM_EFFECT_SETTINGS
    };

    BaseSlider* volumeSlider[2];

    BaseTexture* texMsgBackground;

    BaseTexture* texMessages[NUM_EFFECT_SETTINGS+2];
    SDL_Rect    rects[NUM_EFFECT_SETTINGS+2];
    bool* settings[NUM_EFFECT_SETTINGS];

    BaseTexture* texButtons;

    int *volumeLevel[2];


    int getColumnOffset(int column);
    int buttonWidth;

    void initControls();
    void loadImages();

public:
    EffectsPage(std::string titleStr, const FPoint& pos, const SDL_Rect& rect,bool *bgEffectPtr, bool *translucencyPtr, bool *explodePtr, bool *shinePtr, bool *complexityPtr, bool *trailsPtr,bool *flashPtr,int* musicVolume, int* sfxVolume);
    ~EffectsPage();

    void render(float alphaModifier =1.);
    bool handleEvents();
  //  void renderTitle(float alphaModifier=1.);

};


//=====================================================

class InfoPage: public ScrollPage
{
    enum INFO_SETTINGS
    {
        SETTING_SCORE=0,
        SETTING_TIME,
        SETTING_SHOTS,
        SETTING_BEST,
        SETTING_STATS,
        SETTING_BEATEN,
        SETTING_SWIPEHINT,
        SETTING_OVERRIDE,
        NUM_INFO_SETTINGS
    };

    BaseTexture* texMsgBackground;

    BaseTexture* texMessages[NUM_INFO_SETTINGS];
    SDL_Rect    rects[NUM_INFO_SETTINGS];
    bool* settings[NUM_INFO_SETTINGS];

    BaseTexture* texButtons;

    int getColumnOffset(int column);
    int buttonWidth;

    void initControls();
    void loadImages();

public:
    InfoPage(std::string titleStr, const FPoint& pos, const SDL_Rect& rect,bool *scorePtr, bool* timePtr, bool *shotsPtr, bool *bestPtr, bool *statsPtr, bool *beatenPtr, bool *swipePtr, bool *overPtr );
    ~InfoPage();

    void render(float alphaModifier =1.);
    bool handleEvents();
 //   void renderTitle(float alphaModifier=1.);

};
#endif // OPTIONS_MENU_H
