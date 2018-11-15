#include "base.h"
#include "gameSettings.h"
#include "levelStats.h"
#include "texture.h"
#include "basefuncs.h"

#include <cstdio>

const int NUM_SETTINGS=30;//saved settings

GameSettings::GameSettings()
{
    loadDefaults();
}

GameSettings::~GameSettings()
{
}

void GameSettings::loadDefaults()
{
    skin=SKIN_LIGHT;

    randomness=RANDOMNESS_LOW;
    pressure=PRESSURE_NONE;

    matchNumber=3;
    useBallIndex=true;

    useBombs=true;
    useChromeBalls=true;
    useShifters=true;
    usePainters=true;
    useLevelers=true;


    renderBackgroundEffect=true;
    translucentBackgrounds=true;
    useShineEffect=true;
    fullSceneComplexity=false;
    multiBallExplosions=false;
    renderTrails=false;
    flashOnExplosions=true;

    showRoundScore   = true;
    showRoundTime    = true;
    showRoundShots   = true;
    showBest         = true;
    showStatsOnPause = true;

    musicVolume = 20;
    sfxVolume   = 20;

    runBenchmark=true;
    performanceRating= 255;

    wellWidthID=0;
    showBeatenStats = true;
    showSwipeHint = true;

    overrideSettingsOnContinue = false;

    setColors();
}

//---------------------------------------------------------------

void GameSettings::loadSettings()
{
    SDL_RWops *in;

    char filename[300];

    sprintf(filename,"%s/%s.cfg",glbAppPath,playerStats->getPlayerName().c_str());

    in =SDL_RWFromFile(filename,"r");

    if (!in)
    {
    #ifdef CONSOLE_LOG
        sprintf(glbLogMsg,"ERROR READING %s\n",filename);
        mySDL_Log(glbLogMsg);
    #endif
        loadDefaults();
        return;
    }

    unsigned char buffer[NUM_SETTINGS]={};// init to  zero


    buffer[28]=1;//swipe help should be on by default for existing profiles.


    SDL_RWread(in,buffer,sizeof(char),NUM_SETTINGS);
    SDL_RWclose(in);

    skin                    = buffer[0];
    randomness              = buffer[1];
    pressure                = buffer[2];
    matchNumber             = buffer[3];
    useBallIndex            = buffer[4];
    useBombs                = buffer[5];
    useChromeBalls          = buffer[6];
    renderBackgroundEffect  = buffer[7];
    translucentBackgrounds  = buffer[8];
    useShineEffect          = buffer[9];
    multiBallExplosions     = buffer[10];
    showRoundScore          = buffer[11];
    showRoundTime           = buffer[12];
    showRoundShots          = buffer[13];
    showBest                = buffer[14];
    showStatsOnPause        = buffer[15];
    musicVolume             = buffer[16];
    sfxVolume               = buffer[17];
    useShifters             = buffer[18];
    usePainters             = buffer[19];
    useLevelers             = buffer[20];
    fullSceneComplexity     = buffer[21];
    renderTrails            = buffer[22];
    performanceRating       = buffer[23];
    runBenchmark            = buffer[24];
    flashOnExplosions       = buffer[25];
    wellWidthID             = buffer[26];
    showBeatenStats         = buffer[27];
    showSwipeHint           = buffer[28];
    overrideSettingsOnContinue=buffer[29];

    setColors();

    checkUnlockableSettings();

    #ifdef CONSOLE_LOG
        sprintf(glbLogMsg,"Settings loadedd");
        mySDL_Log(glbLogMsg);
    #endif
}

//----------------------------------------------------

void GameSettings::checkUnlockableSettings()
{
    if ((matchNumber>3) &&(!playerStats->getUnlockableStatus(UNL_MATCH4 + matchNumber-3)) )
        matchNumber=3;

    if ((wellWidthID>DIM_8) &&( !playerStats->getUnlockableStatus(UNL_DIM10 + wellWidthID-DIM_10)) )
        wellWidthID=DIM_8;

    if ((skin > SKIN_GEOMETRIC) &&(! playerStats->getUnlockableStatus(UNL_SKIN_DARK2 + skin-(SKIN_DARK2))) )
        skin-=SKIN_DARK2;

    if ((randomness == RANDOMNESS_HIGHEST) &&(!playerStats->getUnlockableStatus(UNL_TRUERANDOM) ) )
        randomness = RANDOMNESS_HIGH;

    if ((pressure == PRESSURE_HIGHEST) &&(!playerStats->getUnlockableStatus(UNL_CHAOS) ) )
        pressure = PRESSURE_HIGH;

}

//---------------------------------------------------

void GameSettings::setSkin(int newSkin)
{
        glbTexManager->reset();
        skin=newSkin;
        setColors();
}

void GameSettings::writeSettings()
{
    SDL_RWops *out;

    char filename[300];

    sprintf(filename,"%s/%s.cfg",glbAppPath,playerStats->getPlayerName().c_str());

    out =SDL_RWFromFile(filename,"w");

    if (!out)
    {
    #ifdef CONSOLE_LOG
        sprintf(glbLogMsg,"ERROR CREATING %s\n",filename);
        mySDL_Log(glbLogMsg);
    #endif
        return;
    }

    unsigned char buffer[NUM_SETTINGS];

    buffer[0] = skin;
    buffer[1] = randomness;
    buffer[2] = pressure;
    buffer[3] = matchNumber;
    buffer[4] = useBallIndex;
    buffer[5] = useBombs;
    buffer[6] = useChromeBalls;
    buffer[7] = renderBackgroundEffect;
    buffer[8] = translucentBackgrounds;
    buffer[9] = useShineEffect;
    buffer[10] = multiBallExplosions;
    buffer[11] = showRoundScore;
    buffer[12] = showRoundTime;
    buffer[13] = showRoundShots;
    buffer[14] = showBest;
    buffer[15] = showStatsOnPause;
    buffer[16] = musicVolume;
    buffer[17] = sfxVolume;
    buffer[18] = useShifters;
    buffer[19] = usePainters;
    buffer[20] = useLevelers;
    buffer[21] = fullSceneComplexity;
    buffer[22] = renderTrails;
    buffer[23] = performanceRating;
    buffer[24] = runBenchmark;
    buffer[25] = flashOnExplosions;
    buffer[26] = wellWidthID;
    buffer[27] = showBeatenStats;
    buffer[28] = showSwipeHint;
    buffer[29] = overrideSettingsOnContinue;

    SDL_RWwrite(out,buffer,sizeof(char),NUM_SETTINGS);
    SDL_RWclose(out);

    #ifdef CONSOLE_LOG

        sprintf(glbLogMsg,"Settings saved");
        mySDL_Log(glbLogMsg);
    #endif
}


void GameSettings::modulateBGColor(SDL_Color& modColor)
{
    gameSettings->colors[COLOR_BACKGROUND]={
                                            gameSettings->colors[COLOR_ORIGINAL_BACKGROUND].r*(modColor.r/255.),
                                            gameSettings->colors[COLOR_ORIGINAL_BACKGROUND].g*(modColor.g/255.),
                                            gameSettings->colors[COLOR_ORIGINAL_BACKGROUND].b*(modColor.b/255.)
                                            };
}

void GameSettings::resetBGColor()
{
    colors[ COLOR_BACKGROUND ] = colors[ COLOR_ORIGINAL_BACKGROUND];
}

void GameSettings::setColors()
{
    switch(skin)
    {
        case SKIN_DARK:

                colors[COLOR_ORIGINAL_BACKGROUND]       = {  52,  13, 134 };//background mean value
                colors[COLOR_WELL_BACKGROUND]           = {  182,  172, 47 };//background mean value
                colors[COLOR_TIMESTOP_BACKGROUND]       = {  35,  35, 35 };//background when time is stopped
                colors[COLOR_MAXVARIATION]              = {  40,  40, 40 };//max background variation per color

                colors[ COLOR_PAGE_TITLE ]              = { 205, 200, 100 };//scroll page titles

                colors[ COLOR_INGAME_MESSAGE ]          = { 255, 255, 255 };//get ready
                colors[ COLOR_INGAME_BGSTAT ]           = { 0, 0, 0 };
                colors[ COLOR_INGAME_BGBEST ]           = { 130, 20,200 };
                colors[ COLOR_INGAME_BORDERSTAT ]       = { 215, 215, 215 };
                colors[ COLOR_INGAME_BORDERBEST ]       = { 210, 70,255 };//

                colors[ COLOR_NOBG1 ]                   = { 255, 000, 000 };//about category title
                colors[ COLOR_NOBG2 ]                   = { 255, 255, 255 };//about names
                colors[ COLOR_NOBG3 ]                   = { 000, 220, 220 };//about links
                colors[ COLOR_NOBG4 ]                   = { 180, 20, 180 };//title Welcome

                colors[ COLOR_TEXT1 ]                   = { 0, 0, 100 };//profile-stats,  pause-stats
                colors[ COLOR_TEXT2 ]                   = { 135,0, 225 };//{ 210, 90, 00 };//pause-settings, endgame-tip
                colors[ COLOR_TEXT3 ]                   = { 0, 125, 125 };//endgame-new record, pause-best
                colors[ COLOR_TEXT4 ]                   = { 0, 130, 20 };//endgame-pause-comment line under title

                colors[ COLOR_WARNING ]                 = { 0, 180, 230 };//profiles
                break;

        case SKIN_LIGHT:
                colors[COLOR_ORIGINAL_BACKGROUND]       = { 197, 147,  79 };
                colors[COLOR_WELL_BACKGROUND]           = {  178,  159, 244 };//background mean value
                colors[COLOR_TIMESTOP_BACKGROUND]       = {  50,  50, 50 };//background when time is stopped
                colors[COLOR_MAXVARIATION]              = {  20,  40, 30 };//max background variation per color

                colors[ COLOR_PAGE_TITLE ]              = { 255, 255, 255 };//scrollpage titles

                colors[ COLOR_INGAME_MESSAGE ]          = { 255, 255, 255 };//get ready
                colors[ COLOR_INGAME_BGSTAT ]           = { 0, 0, 0 };
                colors[ COLOR_INGAME_BGBEST ]           = { 200, 0,100 };
                colors[ COLOR_INGAME_BORDERSTAT ]       = { 255, 255, 255 };
                colors[ COLOR_INGAME_BORDERBEST ]       = { 255, 100, 255 };

                colors[ COLOR_NOBG1 ]                   = { 190, 0, 100 };//about categorytitle
                colors[ COLOR_NOBG2 ]                   = { 255, 255, 255 };//about names, progress unlocked features
                colors[ COLOR_NOBG3 ]                   = { 40, 230, 255 };//about links
                colors[ COLOR_NOBG4 ]                   = { 180, 20, 180 };//title Welcome

                colors[ COLOR_TEXT1 ]                   = { 255,80,130 };//profile-stats,  pause-stats
                colors[ COLOR_TEXT2 ]                   = { 0, 130, 255 };//pause-settings, endgame-tip
                colors[ COLOR_TEXT3 ]                   = { 255, 50, 200 };//endgame-new record, pause-best
                colors[ COLOR_TEXT4 ]                   = { 100, 50, 255 };//endgame-pause-comment line under title

                colors[ COLOR_WARNING ]                 = {255,255,0};//{ 100, 0, 190 };//profiles

                break;

            case SKIN_PATTERN:
                colors[COLOR_ORIGINAL_BACKGROUND]       = { 113, 144,  87 };
                colors[COLOR_WELL_BACKGROUND]           = {  179,  139, 246 };//background mean value
                colors[COLOR_TIMESTOP_BACKGROUND]       = {  50,  50, 50 };//background when time is stopped
                colors[COLOR_MAXVARIATION]              = {  40,  40, 40 };//max background variation per color

                colors[ COLOR_PAGE_TITLE ]              = { 255, 255, 255 };//scrollpage titles

                colors[ COLOR_INGAME_MESSAGE ]          = { 255, 255, 255 };//get ready
                colors[ COLOR_INGAME_BGSTAT ]           = { 0, 0, 0 };
                colors[ COLOR_INGAME_BGBEST ]           = { 0, 255,0 };
                colors[ COLOR_INGAME_BORDERSTAT ]       = { 255, 255, 255 };
                colors[ COLOR_INGAME_BORDERBEST ]       = {0, 255, 0 };

                colors[ COLOR_NOBG1 ]                   = { 255, 120, 0 };//about categorytitle
                colors[ COLOR_NOBG2 ]                   = { 255, 255, 255 };//about names
                colors[ COLOR_NOBG3 ]                   = { 255, 240, 0 };//about links
                colors[ COLOR_NOBG4 ]                   = { 255, 180, 80 };//title Welcome
                colors[ COLOR_TEXT1 ]                   = {255,60,0};//80,10};//100,30 };//profile-stats,  pause-stats
                colors[ COLOR_TEXT2 ]                   = { 50, 100, 255 };//pause-settings, endgame-tip
                colors[ COLOR_TEXT3 ]                   = { 0, 160, 20 };//endgame-new record, pause-best
                colors[ COLOR_TEXT4 ]                   = { 0, 55, 120 };//endgame-pause-comment line under title

                colors[ COLOR_WARNING ]                 = { 0, 0, 220 };//profiles

                break;

          case SKIN_GEOMETRIC:
                colors[COLOR_ORIGINAL_BACKGROUND]       = { 119, 165, 213 };
                colors[COLOR_WELL_BACKGROUND]           = {  245,  142, 142 };//background mean value
                colors[COLOR_TIMESTOP_BACKGROUND]       = {  50,  50, 50 };//background when time is stopped
                colors[COLOR_MAXVARIATION]              = {  40,  30, 0 };//max background variation per color

                colors[ COLOR_PAGE_TITLE ]              = { 255, 255, 255 };//scrollpage titles

                colors[ COLOR_INGAME_MESSAGE ]          = { 255, 255, 255 };//get ready
                colors[ COLOR_INGAME_BGSTAT ]           = { 0, 0, 0 };
                colors[ COLOR_INGAME_BGBEST ]           = { 0, 255,0 };
                colors[ COLOR_INGAME_BORDERSTAT ]       = { 255, 255, 255 };
                colors[ COLOR_INGAME_BORDERBEST ]       = {0, 255, 0 };

                colors[ COLOR_NOBG1 ]                   = { 0, 75, 255 };//about categorytitle
                colors[ COLOR_NOBG2 ]                   = { 255, 255, 255 };//about names
                colors[ COLOR_NOBG3 ]                   = { 0, 255, 255 };//about links
                colors[ COLOR_NOBG4 ]                   = { 0, 100, 250 };//title Welcome

                colors[ COLOR_TEXT1 ]                   = {140,20,255 };//profile-stats,  pause-stats
                colors[ COLOR_TEXT2 ]                   = { 30, 70, 255 };//pause-settings, endgame-tip
                colors[ COLOR_TEXT3 ]                   = { 20, 130, 0 };//endgame-new record, pause-best
                colors[ COLOR_TEXT4 ]                   = { 125,0, 175 };//{ 255,65, 0 };//endgame-pause-comment line under title

                colors[ COLOR_WARNING ]                 = { 0, 0, 220 };//profiles

                break;

        case SKIN_DARK2:

                colors[COLOR_ORIGINAL_BACKGROUND]       = {  38,  28, 78 }; //background mean value
                colors[COLOR_WELL_BACKGROUND]           = {  40,  136, 160 }; //background mean value
                colors[COLOR_TIMESTOP_BACKGROUND]       = {  35,  35, 35 };//background when time is stopped
                colors[COLOR_MAXVARIATION]              = {  40,  40, 40 };//max background variation per color

                colors[ COLOR_PAGE_TITLE ]              = { 205, 200, 100 };//scroll page titles

                colors[ COLOR_INGAME_MESSAGE ]          = { 255, 255, 255 };//get ready
                colors[ COLOR_INGAME_BGSTAT ]           = { 0, 0, 0 };
                colors[ COLOR_INGAME_BGBEST ]           = { 130, 20,200 };
                colors[ COLOR_INGAME_BORDERSTAT ]       = { 215, 215, 215 };
                colors[ COLOR_INGAME_BORDERBEST ]       = { 210, 70,255 };//

                colors[ COLOR_NOBG1 ]                   = { 255, 000, 000 };//about category title
                colors[ COLOR_NOBG2 ]                   = { 255, 255, 255 };//about names
                colors[ COLOR_NOBG3 ]                   = { 000, 220, 220 };//about links
                colors[ COLOR_NOBG4 ]                   = { 180, 20, 180 };//title Welcome

                colors[ COLOR_TEXT1 ]                   = { 0, 0, 100 };//profile-stats,  pause-stats
                colors[ COLOR_TEXT2 ]                   = { 135,0, 225 };//{ 210, 90, 00 };//pause-settings, endgame-tip
                colors[ COLOR_TEXT3 ]                   = { 0, 125, 125 };//endgame-new record, pause-best
                colors[ COLOR_TEXT4 ]                   = { 0, 130, 20 };//endgame-pause-comment line under title

                colors[ COLOR_WARNING ]                 = { 0, 180, 230 };//profiles
                break;

        case SKIN_LIGHT2:
                colors[COLOR_ORIGINAL_BACKGROUND]       = { 200, 92,  183 };
                colors[COLOR_WELL_BACKGROUND]           = {  73,  177, 236 };//background mean value
                colors[COLOR_TIMESTOP_BACKGROUND]       = {  50,  50, 50 };//background when time is stopped
                colors[COLOR_MAXVARIATION]              = {  10,  20, 40 };//max background variation per color

                colors[ COLOR_PAGE_TITLE ]              = { 255, 255, 255 };//scrollpage titles

                colors[ COLOR_INGAME_MESSAGE ]          = { 255, 255, 255 };//get ready
                colors[ COLOR_INGAME_BGSTAT ]           = { 0, 0, 0 };
                colors[ COLOR_INGAME_BGBEST ]           = { 200, 0,100 };
                colors[ COLOR_INGAME_BORDERSTAT ]       = { 255, 255, 255 };
                colors[ COLOR_INGAME_BORDERBEST ]       = { 255, 100, 255 };

                colors[ COLOR_NOBG1 ]                   = { 100, 0, 220 };//about categorytitle
                colors[ COLOR_NOBG2 ]                   = { 255, 255, 255 };//about names, progress unlocked features
                colors[ COLOR_NOBG3 ]                   = { 40, 230, 255 };//about links
                colors[ COLOR_NOBG4 ]                   = { 180, 20, 180 };//title Welcome

                colors[ COLOR_TEXT1 ]                   = { 255,80,130 };//profile-stats,  pause-stats
                colors[ COLOR_TEXT2 ]                   = { 0, 130, 255 };//pause-settings, endgame-tip
                colors[ COLOR_TEXT3 ]                   = { 255, 50, 200 };//endgame-new record, pause-best
                colors[ COLOR_TEXT4 ]                   = { 100, 50, 255 };//endgame-pause-comment line under title

                colors[ COLOR_WARNING ]                 = {255,255,0};//{ 100, 0, 190 };//profiles

                break;

            case SKIN_PATTERN2:
                colors[COLOR_ORIGINAL_BACKGROUND]       = { 165, 118,  187 };
                colors[COLOR_WELL_BACKGROUND]           = {  255,  160, 27 };//background mean value
                colors[COLOR_TIMESTOP_BACKGROUND]       = {  50,  50, 50 };//background when time is stopped
                colors[COLOR_MAXVARIATION]              = {  40,  40, 40 };//max background variation per color

                colors[ COLOR_PAGE_TITLE ]              = { 255, 255, 255 };//scrollpage titles

                colors[ COLOR_INGAME_MESSAGE ]          = { 255, 255, 255 };//get ready
                colors[ COLOR_INGAME_BGSTAT ]           = { 0, 0, 0 };
                colors[ COLOR_INGAME_BGBEST ]           = { 0, 255,0 };
                colors[ COLOR_INGAME_BORDERSTAT ]       = { 255, 255, 255 };
                colors[ COLOR_INGAME_BORDERBEST ]       = {0, 255, 0 };

                colors[ COLOR_NOBG1 ]                   = { 255, 100, 0 };//about categorytitle
                colors[ COLOR_NOBG2 ]                   = { 255, 255, 255 };//about names
                colors[ COLOR_NOBG3 ]                   = { 255, 240, 0 };//about links
                colors[ COLOR_NOBG4 ]                   = { 100, 100, 255 };//title Welcome
                colors[ COLOR_TEXT1 ]                   = {255,60,0};//80,10};//100,30 };//profile-stats,  pause-stats
                colors[ COLOR_TEXT2 ]                   = { 50, 100, 255 };//pause-settings, endgame-tip
                colors[ COLOR_TEXT3 ]                   = { 0, 160, 20 };//endgame-new record, pause-best
                colors[ COLOR_TEXT4 ]                   = { 0, 55, 120 };//endgame-pause-comment line under title

                colors[ COLOR_WARNING ]                 = { 0, 0, 220 };//profiles

                break;

          case SKIN_GEOMETRIC2:
                colors[COLOR_ORIGINAL_BACKGROUND]       = { 110, 131, 255 };//110,131
                colors[COLOR_WELL_BACKGROUND]           = {  79,  128, 219 };//background mean value
                colors[COLOR_TIMESTOP_BACKGROUND]       = {  50,  50, 50 };//background when time is stopped
                colors[COLOR_MAXVARIATION]              = {  40,  10, 0 };//max background variation per color

                colors[ COLOR_PAGE_TITLE ]              = { 255, 255, 255 };//scrollpage titles

                colors[ COLOR_INGAME_MESSAGE ]          = { 255, 255, 255 };//get ready
                colors[ COLOR_INGAME_BGSTAT ]           = { 0, 0, 0 };
                colors[ COLOR_INGAME_BGBEST ]           = { 0, 255,0 };
                colors[ COLOR_INGAME_BORDERSTAT ]       = { 255, 255, 255 };
                colors[ COLOR_INGAME_BORDERBEST ]       = {0, 255, 0 };

                colors[ COLOR_NOBG1 ]                   = { 0, 75, 255 };//about categorytitle
                colors[ COLOR_NOBG2 ]                   = { 255, 255, 255 };//about names
                colors[ COLOR_NOBG3 ]                   = { 0, 255, 255 };//about links
                colors[ COLOR_NOBG4 ]                   = { 0, 100, 250 };//title Welcome

                colors[ COLOR_TEXT1 ]                   = {140,20,255 };//profile-stats,  pause-stats
                colors[ COLOR_TEXT2 ]                   = { 30, 70, 255 };//pause-settings, endgame-tip
                colors[ COLOR_TEXT3 ]                   = { 20, 130, 0 };//endgame-new record, pause-best
                colors[ COLOR_TEXT4 ]                   = { 125,0, 175 };//{ 255,65, 0 };//endgame-pause-comment line under title

                colors[ COLOR_WARNING ]                 = { 0, 0, 220 };//profiles

                break;


    }

    resetBGColor();

}
int GameSettings::getWellWidthValue()
{
    return getWidthValueByIndex(wellWidthID);
}

int GameSettings::getWidthValueByIndex(int widthID)
{
    return BASE_WELL_WIDTH + widthID*2;
}
