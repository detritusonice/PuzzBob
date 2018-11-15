#ifndef GAME_SETTINGS_H
#define GAME_SETTINGS_H

#include "constants.h"

class GameSettings
{

public:
    int skin;

    int randomness;
    int pressure;
    int matchNumber;

    bool useBallIndex;
    bool useBombs;
    bool useChromeBalls;
    bool useShifters;
    bool usePainters;
    bool useLevelers;

    bool renderBackgroundEffect;
    bool useShineEffect;
    bool multiBallExplosions;
    bool translucentBackgrounds;
    bool fullSceneComplexity;
    bool renderTrails;
    bool flashOnExplosions;

    bool showRoundScore;
    bool showRoundTime;
    bool showRoundShots;
    bool showBest;
    bool showStatsOnPause;

    bool showBeatenStats;

    int musicVolume;
    int sfxVolume;

    unsigned char performanceRating;
    bool runBenchmark;

    int wellWidthID;

    bool showSwipeHint;
    bool overrideSettingsOnContinue;

    SDL_Color colors[NUM_TEXTCOLORS];

    GameSettings();
    ~GameSettings();

    void loadDefaults();
    void loadSettings();
    void writeSettings();
    void setColors();
    void modulateBGColor(SDL_Color& modColor);
    void resetBGColor();

    void checkUnlockableSettings();

    int getWellWidthValue();
    int getWidthValueByIndex(int widthID);

    void setSkin(int newSkin);
};
#endif // GAME_SETTINGS_H
