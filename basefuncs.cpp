#include "base.h"
#include "cursor.h"
#include "basefuncs.h"
#include "globals.h"
#include "states.h"
#include "screenEffects.h"
#include "texture.h"
#include "levelStats.h"
#include "randomGenerator.h"
#include "myMath.h"
#include "profiler.h"
#include "gameSettings.h"
#include "antihack.h"

#include <cstdio>
#include <cstring>
#include <ctime>

#include <string>


SDL_Rect* screenRect;
SDL_Rect* guiBox;

TextureManager* glbTexManager;
ScreenEffect* glbEffect;
SDL_RWops *glbLogFile;

Mix_Music* glbMusic;

RandomGenerator* randGen;
GameSettings* gameSettings;

Mix_Chunk* glbSfx[NUM_SOUND_EFFECTS];

int refreshRate;

std::string prevMusicFile;


LevelStats* playerStats;

float guiUnit;
char glbLogMsg[500];

Profiler* glbProfiler;

char glbAppPath[500];

//-----------------------------------------------------------------------------------
void setScreenRect(int width, int height)
{
    screenRect->x=0;
    screenRect->y=0;
    screenRect->w=width;
    screenRect->h=height;

    float minDim = std::min ( 1.*screenRect->w/GUI_UNIT_W , 1.*screenRect->h/GUI_UNIT_H );

    guiUnit=minDim;

    guiBox->x=(screenRect->w - guiUnit*GUI_UNIT_W)/2;
    guiBox->y=(screenRect->h - guiUnit*GUI_UNIT_H)/2;
    guiBox->w=GUI_UNIT_W*guiUnit;
    guiBox->h=GUI_UNIT_H*guiUnit;


}
//-------------------------------------------------------------------------------------
int genRand10000()
{
    return randGen->generateNext();
}
//------------------------------------------------------------------------------------
std::string loadLastPlayer()
{
    std::string filename;

    filename=glbAppPath;

    filename += "/profiles.bin";//.txt

    char namebuffer[MAX_PROFILENAME_LENGTH+1];
    int lastUsed=0;

    SDL_RWops *in=NULL;

    in = SDL_RWFromFile(filename.c_str(),"r");

    if (in==NULL)
    {
        return "Player";
    }
    int numProfiles=0;

    SDL_RWread(in,&numProfiles,sizeof(int),1);

    while (numProfiles--)
    {
        SDL_RWread(in,namebuffer,sizeof(char),MAX_PROFILENAME_LENGTH);
        SDL_RWread(in,&lastUsed,sizeof(int),1);
        if (lastUsed==0)
            break;
    }
    SDL_RWclose(in);
    return std::string(namebuffer);
}
//-------------------------------------------------------------------------------------

bool init()
{
    #ifdef PLATFORM_ANDROID
        strcpy(glbAppPath,SDL_AndroidGetInternalStoragePath());
    #else
        strcpy(glbAppPath,".");
    #endif

    #ifdef PROFILING
        glbProfiler = new Profiler();
    #endif // PROFILING



    initLogFile();
#ifdef CONSOLE_LOG
    sprintf(glbLogMsg,"Starting...\n"); mySDL_Log(glbLogMsg);
#endif


    screenRect= new SDL_Rect();
    guiBox = new SDL_Rect();

    setScreenRect(480,800);

    if ( SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER/*| SDL_INIT_JOYSTICK*/) == -1 )
    {
        sprintf(glbLogMsg,"error initializing SDL %s\n",SDL_GetError());
        mySDL_Log(glbLogMsg);
        return false;
    }



    refreshRate=60;//default screen refresh rate

    SDL_DisplayMode displayMode;
    if (SDL_GetCurrentDisplayMode(0,&displayMode)==0)
    {
        SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1);
    #ifdef PLATFORM_ANDROID
        SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 0);
        SDL_GL_SetAttribute(SDL_GL_RETAINED_BACKING, 0);//screen problem on some mobile devices if these are not set
        setScreenRect(displayMode.w, displayMode.h);
    #else
        if (displayMode.refresh_rate >0)
            refreshRate=displayMode.refresh_rate;
    #endif
    }

    char str[40];

    sprintf(str,"PuzzBob v%.3f%c",PROGRAM_VERSION,isLegitimateCopy?' ':'f');//in demo version legitimate copy is false

    glbWindow = SDL_CreateWindow( str,SDL_WINDOWPOS_UNDEFINED,SDL_WINDOWPOS_UNDEFINED,
                                  screenRect->w,screenRect->h,SDL_WINDOW_SHOWN );//| SDL_WINDOW_FULLSCREEN_DESKTOP );
    if (glbWindow ==NULL)
    {
        sprintf(glbLogMsg,"Error creating glbWindow: %s\n",SDL_GetError()); return false;
        mySDL_Log(glbLogMsg);
    }

    int w,h;
    SDL_GetWindowSize(glbWindow,&w,&h);
    setScreenRect(w,h);

    glbRenderer = SDL_CreateRenderer(glbWindow,-1,SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (glbRenderer == NULL)
    {
        sprintf(glbLogMsg,"Error creating glbRenderer: %s\n",SDL_GetError());
        mySDL_Log(glbLogMsg);
        return false;
    }
//    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
//    SDL_RenderSetLogicalSize(glbRenderer,w/2,h/2);

    SDL_SetRenderDrawColor(glbRenderer,0x00,0x00,0x00,0xFF);
    SDL_RenderClear(glbRenderer);
    SDL_RenderPresent(glbRenderer);


    if ((IMG_Init(IMG_INIT_PNG ) & (IMG_INIT_PNG)) ==0)
    {
        sprintf(glbLogMsg,"Error initializing SDL Image: %s\n",IMG_GetError());
        mySDL_Log(glbLogMsg);
        return false;
    }

    if ( TTF_Init() == -1 )
    {
        sprintf(glbLogMsg,"Error initializing SDL TTF: %s\n",TTF_GetError());
        mySDL_Log(glbLogMsg);
        return false;
    }

    if ( Mix_OpenAudio( 44100, MIX_DEFAULT_FORMAT, 2, 2048)==-1)
    {
        sprintf(glbLogMsg,"Mix_OpenAudio: %s\n", Mix_GetError());
        mySDL_Log(glbLogMsg);
        return false;
    }


    glbGameCursor=InitGameCursor(CURSOR_CROSSHAIR);
    SDL_SetCursor( glbGameCursor );

    SDL_ShowCursor(SDL_DISABLE);


    randGen = new RandomGenerator(0,10000,(unsigned)SDL_GetTicks());

#ifdef HACK_PROTECTION
    #ifdef PLATFORM_PC
       ahGetDexSum("F:/DEV/code/TRUNK/code/SDL/Android/SDL2-2.0.3/android-project/bin/classes.dex");
       sprintf(glbLogMsg,"Checksum written in DEXSum.txt\n");
       mySDL_Log(glbLogMsg);
    #endif // PLATFORM_PC
#endif

    std::string lastPlayer=loadLastPlayer();


    playerStats = new LevelStats(lastPlayer);

    gameSettings= new GameSettings();
    gameSettings->loadSettings();

    glbTexManager = new TextureManager();

    glbEffect = new AirBubbles();

    Mix_VolumeMusic(gameSettings->musicVolume);
    Mix_Volume(-1,gameSettings->sfxVolume);

    glbMusic=NULL;
    prevMusicFile="";

    loadSoundEffects();
    setSFXVolume(gameSettings->sfxVolume);

    return true;
}

//-----------------------------------------------------------------------------------

void cleanUp()
{
    Mix_HaltMusic();
    Mix_HaltChannel(-1);

    if (glbMusic!=NULL)
    {
        Mix_FreeMusic(glbMusic);
        glbMusic=NULL;
    }

    for (int i=0;i<NUM_SOUND_EFFECTS;i++)
        if (glbSfx[i]!=NULL)
        {
            Mix_FreeChunk(glbSfx[i]);
            glbSfx[i]=NULL;
        }

    Mix_CloseAudio();//atexit calls it


    delete playerStats;

#ifdef PROFILING
    delete glbProfiler;
#endif // PROFILING
    delete gameSettings;
    delete randGen;

    delete glbEffect;
    delete glbTexManager;

    if(glbFont!=NULL) TTF_CloseFont( glbFont );
    if(glbMessageFont!=NULL) TTF_CloseFont( glbMessageFont );

    if (glbGameCursor!=NULL)
        SDL_FreeCursor(glbGameCursor);

    if (glbRenderer !=NULL) {SDL_DestroyRenderer(glbRenderer); glbRenderer = NULL;}

    if (glbWindow != NULL ) {SDL_DestroyWindow(glbWindow); glbWindow = NULL;}

    delete screenRect;
    delete guiBox;

    closeLogFile();

    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
}

//-----------------------------------------------------------------------------------

bool loadFiles()
{

    glbFont = TTF_OpenFont( "media/gamefont.otf", guiUnit*0.66 );

    if (glbFont==NULL)
    {
#ifdef CONSOLE_LOG
        sprintf(glbLogMsg,"Error loading font :%s\n",TTF_GetError());
        mySDL_Log(glbLogMsg);
#endif
        return false;
    }

    glbMessageFont = TTF_OpenFont( "media/gamefont.otf", guiUnit*0.5 );

    if (glbMessageFont==NULL)
    {
#ifdef CONSOLE_LOG
        sprintf(glbLogMsg,"Error loading font :%s\n",TTF_GetError());
        mySDL_Log(glbLogMsg);
#endif
        return false;
    }

    return true;
}

void initLogFile()
{
#ifdef FILE_LOG
    glbLogFile=NULL;
    char logfname[300];

    sprintf(logfname,"%s/puzzBobLog.txt",glbAppPath);


    glbLogFile =SDL_RWFromFile(logfname,"wt");

    if (!glbLogFile)
    {
    #ifdef CONSOLE_LOG
        SDL_Log("ERROR CREATING %s\n",logfname);
    #endif
    }
    #ifdef CONSOLE_LOG
    else
        SDL_Log(" log file is:%s\n",logfname);
    #endif
#endif

}

void closeLogFile()
{
#ifdef FILE_LOG
    if (glbLogFile!=NULL)
    {
        SDL_RWclose(glbLogFile);
    #ifdef CONSOLE_LOG
        SDL_Log(" closed log file\n");
    #endif
    }
#endif
}

void loadPlayMusic(const char* filename)
{

    std::string newFile=filename;

    if (prevMusicFile==filename)
    {
        setMusicVolume(gameSettings->musicVolume);
        return;
    }

    prevMusicFile=filename;

    if (glbMusic!=NULL)
    {
        if (Mix_PlayingMusic())
            Mix_HaltMusic();
        Mix_FreeMusic(glbMusic);
        glbMusic=NULL;
    }

    glbMusic = Mix_LoadMUS(filename);
    if (glbMusic==NULL)
    {
 #ifdef CONSOLE_LOG
        sprintf(glbLogMsg,"could not load music %s\n",Mix_GetError());
        mySDL_Log(glbLogMsg);
#endif
    }
    else
    {
        Mix_PlayMusic(glbMusic,-1);
        setMusicVolume(gameSettings->musicVolume);
    }
}

void setMusicVolume (int volumeLevel)
{
    Mix_VolumeMusic(volumeLevel);

    if (glbMusic!=NULL)
    {
        if (volumeLevel==0)
        {
            if (Mix_PlayingMusic())
                Mix_PauseMusic();
        }
        else
        {
            if (Mix_PausedMusic())
                Mix_ResumeMusic();
        }
    }
}

void setSFXVolume(int volumeLevel)
{
    for (int i=0;i<NUM_SOUND_EFFECTS;i++)
        if (glbSfx[i]!=NULL)
            Mix_VolumeChunk(glbSfx[i],volumeLevel);

}

bool loadSoundEffects()
{
    const char* sfxFiles[]={
        "media/Sounds/tap1.ogg",//SFX_TAP1
        "media/Sounds/tap2.ogg",//SFX_TAP2
        "media/Sounds/page.ogg",//SFX_PAGESCROLL
        "media/Sounds/menupop.ogg",//SFX_MENUPOP
        "media/Sounds/swap.ogg",//SFX_SWAPCOLOR
        "media/Sounds/bubble.ogg",//SFX_BUBBLE
        "media/Sounds/launch.ogg",//SFX_LAUNCH
        "media/Sounds/launch2.ogg",//SFX_LAUNCH
        "media/Sounds/collision.ogg",//SFX_COLLISION
        "media/Sounds/collision2.ogg",//SFX_COLLISION2
        "media/Sounds/land.ogg",//SFX_LAND
        "media/Sounds/land2.ogg",//SFX_LAND2
        "media/Sounds/landfail.ogg",//SFX_LANDFAIL
        "media/Sounds/drop.ogg",//SFX_DROP
        "media/Sounds/drop2.ogg",//SFX_DROP2
        "media/Sounds/drop3.ogg",//SFX_DROP3
        "media/Sounds/drop4.ogg",//SFX_DROP4
        "media/Sounds/pop.ogg",//SFX_POP
        "media/Sounds/pop2.ogg",//SFX_POP2
        "media/Sounds/pop3.ogg",//SFX_POP3
        "media/Sounds/pop4.ogg",//SFX_POP4
        "media/Sounds/pop5.ogg",//SFX_POP5
        "media/Sounds/pop6.ogg",//SFX_POP6
        "media/Sounds/pop7.ogg",//SFX_POP7
        "media/Sounds/burst1.ogg",//SFX_EXPLODE
        "media/Sounds/burst2.ogg",//SFX_EXPLODE2
        "media/Sounds/burst3.ogg",//SFX_EXPLODE4
        "media/Sounds/burst4.ogg",//SFX_EXPLODE4
        "media/Sounds/colorshifter.ogg",//SFX_COLORSHIFTER
        "media/Sounds/painter.ogg",//SFX_PAINTER
        "media/Sounds/bomb.ogg",//SFX_BOMB
        "media/Sounds/leveler.ogg",//SFX_LEVELER
        "media/Sounds/chrome.ogg",//SFX_CHROME
        "media/Sounds/timestop.ogg"//SFX_TIMESTOP
    };

    for (int i=0;i<NUM_SOUND_EFFECTS;i++)
        glbSfx[i]=Mix_LoadWAV(sfxFiles[i]);

    return true;
}

void playSound(int effect,int numLoops)
{
    if ( (gameSettings->sfxVolume!=0) && (glbSfx[effect]!=NULL) )
            Mix_PlayChannel(-1,glbSfx[effect],numLoops);
}

void mySDL_Log( char* str)
{
    SDL_Log(str);
#ifdef FILE_LOG
    int len=strlen(str);
    if (str[len-1]=='\n')
        str[len-1]='\r';
    if (glbLogFile!=NULL)
        SDL_RWwrite( glbLogFile, str, len*sizeof(char),1);
#endif
}


/*
void displaymode()
{

    unsigned int r,g,b,a;
    r=displayMode.format;
    a=r&255;
    r>>=8;
    b=r&255;
    r>>=8;
    g=r&255;
    r>>=8;
    sprintf(glbLogMsg,"DisplayMode: (%d x %d) ,%x =[ %x, %x, %x, %x ], %d Hz \n",displayMode.w,displayMode.h,displayMode.format,r,g,b,a,displayMode.refresh_rate);

#ifdef CONSOLE_LOG
        mySDL_Log(glbLogMsg);
#else
        SDL_Log("%s",glbLogMsg);
#endif

}
*/
