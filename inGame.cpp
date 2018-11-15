#include "base.h"

#include "basefuncs.h"

#include "stateMachine.h"
#include "states.h"
#include "inGame.h"

#include "texture.h"
#include "button.h"
#include "gameMenu.h"
#include "summonMenu.h"
#include "levelSelector.h"

#include "screenEffects.h"
#include "localEffects.h"
#include "randomGenerator.h"
#include "gameSettings.h"
#include "endgame.h"
#include "pauseScreen.h"

#include <cstdio> //for sprintf()
#include <cmath> //for acos()
#include <cstdlib>

#include "myMath.h"

//extern float framesPerSec;

const float SQRT3           = 1.7320508075;
const float MAX_ANGLE       = 80.f;
const int SAVE_BUFFER_SIZE  = 28;

float BALL_VELOCITY;

#ifdef PLATFORM_ANDROID
extern float meanShotTime;
#endif // PLATFORM_ANDROID


//===================   BALLINFO   ========================================
BallInfo::BallInfo( float ballRadius, const FPoint& ballPos, int ballColor, int ballState,bool useEffects) : radius(ballRadius), pos(ballPos), color(ballColor), state(ballState)
{
    DEBUG_REC();

    vel = FPoint(0.,0.);
    acceleration=FPoint(0.,0.);

    alpha =255.f;

    for (int i=0;i<NUM_BALL_EFFECTS;i++)
    {
        effect[i]=NULL;
    }

    if ( (useEffects) && (state!=BALLSTATE_FALLING))//creation when loading a saved state only
    {
        float rnd=genRand10000();

        effect[EFFECT_EXPLOSION] = new ExplosionFlashEffect(pos,4.*radius,color,
                                                       0.6+rnd/20000.,
                                                       1.+rnd/1000.);

        if ( (gameSettings->useShineEffect)&&(color!=COLOR_BLOCK) )
            effect[EFFECT_SHINE] = new ShineEffect(pos,radius);

        effect[EFFECT_SPARKS]= new ExplosionSparksEffect(pos,color,
                                                         radius/3,FPoint(0.,BALL_VELOCITY));

        if (color==COLOR_BOMB)
            effect[EFFECT_FUSE] = new FuseEffect( pos,FPoint(27.,-32).GetNormal()*radius*0.95,
                                                    radius/4.,
                                                    FPoint(0.,BALL_VELOCITY)
                                                     );
        if ((color>COLOR_BOMB) && (color<NUM_BALLCOLORS))
            effect[EFFECT_FUSE] = new PixiesEffect( pos,color,radius/4.);

        //HERE
    }
}
//----------------------------------------------------------
BallInfo::~BallInfo()
{
    DEBUG_REC();

    for (int i=0;i<NUM_BALL_EFFECTS;i++)
    {
        if (effect[i] != NULL)
            delete effect[i];
    }
}
//----------------------------------------------------------
void BallInfo::setPosition(const FPoint& newPos)
{
    DEBUG_REC();

    pos = newPos;
    for (int i=0;i<NUM_BALL_EFFECTS;i++)
    {
        if (effect[i] != NULL)
            effect[i]->setPosition(newPos);
    }
}

void BallInfo::setAcceleration( const FPoint& a)
{
    acceleration = a;
}
//-------------------------------------------------

void BallInfo::setColor(int newColor)
{
    DEBUG_REC();

    if (color == newColor) return;

    if (color >= COLOR_BOMB) //was a bomb
    {
        if (effect[EFFECT_FUSE]!=NULL)
        {
            delete effect[EFFECT_FUSE];
            effect[EFFECT_FUSE]=NULL;
        }
    }

    if (newColor == COLOR_BOMB) //was plain ball, becomes a bomb
    {
        effect[EFFECT_FUSE] = new FuseEffect( pos,FPoint(27.,-32).GetNormal()*radius,
                                                radius/4.,
                                                FPoint(0.,BALL_VELOCITY)
                                                 );
    }
    else
        if ((newColor>COLOR_BOMB) && (newColor<NUM_BALLCOLORS))
            effect[EFFECT_FUSE] = new PixiesEffect( pos,newColor,radius/4.);

            //HEERE FOR ADDITIONAL COLORS

    color = newColor;
    for (int i=0;i<NUM_BALL_EFFECTS;i++)
    {
        if (effect[i] != NULL)
            effect[i]->setColor(newColor);
    }
}
//----------------------------------------------------------
void BallInfo::setState(int newState,int scoreType)
{
    DEBUG_REC();

    state=newState;


    if (state == BALLSTATE_FALLING)
    {
        if (effect[EFFECT_SHINE]!=NULL)
        {
            delete effect[EFFECT_SHINE];
            effect[EFFECT_SHINE]=NULL;
        }

        if (scoreType!=SCORE_BOMB)
        {
            effect[EFFECT_SHINE]=new ScoreEffect(pos,2*radius,scoreType,0.8f);
        }
    }

}
//----------------------------------------------------------
void BallInfo::setVelocity(const FPoint& newVel)
{
    vel=newVel;
}
//----------------------------------------------------------
void BallInfo::setAlpha(int newAlpha)
{
    alpha = std::max( 0, std::min (newAlpha,255));
}
//-----------------------------------------------------------
void BallInfo::fade(float rate)
{
    alpha*=(1.+rate);
}
void BallInfo::setRadius(float newRadius)
{
    radius = newRadius;
}

float BallInfo::getRadius()
{
    return radius;
}
//----------------------------------------------------------
FPoint BallInfo::getPosition()
{
    return pos;
}
//----------------------------------------------------------
FPoint BallInfo::getVelocity()
{
    return vel;
}
//----------------------------------------------------------
int BallInfo::getState()
{
    return state;
}
//----------------------------------------------------------
int BallInfo::getColor()
{
    return color;
}
//----------------------------------------------------------
int BallInfo::getAlpha()
{
    return alpha;
}
//----------------------------------------------------------
void BallInfo::update(float dt)
{
    DEBUG_REC();

    FPoint p=pos;
    p+=vel*dt;
    setPosition(p);

    vel+=acceleration*dt;
}

//----------------------------------------------------------
bool BallInfo::updateEffects(float dt, float phase)
{
    DEBUG_REC();
    bool effectActive=false;

    for (int i=0;i<NUM_BALL_EFFECTS;i++)
    {
        if (effect[i] != NULL)
            effectActive|=effect[i]->update(dt,phase);
    }
    return effectActive;
}
//----------------------------------------------------------
void BallInfo::renderBall( int handle,const SDL_Rect& rc, const FPoint& offset)
{
    //DEBUG_REC();

    int rad=rc.w/2;

    glbTexManager->getTexturePtr( handle )->render(
                                                         pos.x+offset.x-rad,
                                                         pos.y+offset.y-rad,
                                                         &rc,-1,alpha);

}
//----------------------------------------------------------
void BallInfo::renderEffects(bool showShine,bool showOthers, const FPoint& offset)
{
    //DEBUG_REC();

    if ((showShine)&&(!showOthers))
    {
        if (effect[EFFECT_SHINE]!=NULL)
            ((ShineEffect*)effect[EFFECT_SHINE])->render(offset,alpha/255.);
        return;
    }

    for (int i=0;i<NUM_BALL_EFFECTS;i++)
    {
        if (effect[i] == NULL) continue;

        if ((i==EFFECT_SHINE) && (!showShine) )  continue;

        effect[i]->render(alpha/255.);
    }
}
//----------------------------------------------------------
void BallInfo::transformEffects(const FPoint& pt)
{

    for (int i=0;i<NUM_BALL_EFFECTS;i++)
    {
        if (effect[i] != NULL)
            effect[i]->transformEffect(pt);
    }
}
//----------------------------------------------------------
void BallInfo::explode()
{
    DEBUG_REC();

    if (effect[EFFECT_EXPLOSION]!=NULL)
        effect[EFFECT_EXPLOSION]->start();
}
//-----------------------------------------------------------
void BallInfo::explodeWithSparks()
{
    DEBUG_REC();

    explode();

    if (effect[EFFECT_SPARKS]!=NULL)
        effect[EFFECT_SPARKS]->start();
}
//----------------------------------------------------------
void BallInfo::saveState(SDL_RWops* out)
{
    DEBUG_REC();

    SDL_RWwrite(out,&pos,sizeof(FPoint),1);
    SDL_RWwrite(out,&vel,sizeof(FPoint),1);
    SDL_RWwrite(out,&acceleration,sizeof(FPoint),1);
    SDL_RWwrite(out,&color,sizeof(int),1);
    SDL_RWwrite(out,&alpha,sizeof(int),1);
    SDL_RWwrite(out,&state,sizeof(int),1);
}
//----------------------------------------------------------
void BallInfo::loadState( SDL_RWops *in)
{
    DEBUG_REC();

    SDL_RWread(in,&pos,sizeof(FPoint),1);
    SDL_RWread(in,&vel,sizeof(FPoint),1);
    SDL_RWread(in,&acceleration,sizeof(FPoint),1);
    SDL_RWread(in,&color,sizeof(int),1);
    SDL_RWread(in,&alpha,sizeof(int),1);
    SDL_RWread(in,&state,sizeof(int),1);

    for (int i=0;i<NUM_BALL_EFFECTS;i++)
    {
        if (effect[i]!=NULL)
            delete effect[i];
        effect[i]=NULL;
    }
}
//===============================================================================

InGame::InGame()
{
    DEBUG_REC();

#ifdef CONSOLE_LOG
    sprintf(glbLogMsg, "InGame:Constructor\n");
    mySDL_Log(glbLogMsg);
#endif // CONSOLE_LOG

    WELL_WIDTH=gameSettings->getWellWidthValue();
    WELL_HEIGHT=1.75*WELL_WIDTH;

    isHibernating=false;

    trailsEffect=NULL;

    currentBall=NULL;
    nextBall   =NULL;

    largeMessageFont=NULL;
    mediumMessageFont=NULL;
    smallMessageFont=NULL;

    levelSelector=NULL;
    endGame=NULL;
    gameMenu=NULL;
    pauseScreen=NULL;
    summonMenu = NULL;

    //texScore    =new BaseTexture;
    texLevel    =new BaseTexture;
    texMessage  =new BaseTexture;
    texCountDown=new BaseTexture;

    texSummon=NULL;
    summonEffect=NULL;

    BALL_INDEX_LENGTH = 2.5*WELL_HEIGHT;

    for (int i=0;i<3;i++)
    {
        texRoundScore[i]=new BaseTexture;
        texRoundShots[i]=new BaseTexture;
        texRoundTime[i]=new BaseTexture;
    }

    texStaticBalls=NULL;
    texShiningBalls=NULL;
    texBallIndex=NULL;
    texWellHardware=NULL;
    texLauncher=NULL;
    texPaused=NULL;

    for (int i=0;i<NUM_TEXTURES;i++)
            recs[i]=new SDL_Rect;

    piston_step =0;//needed by compute dimensions
    initial_step=0;

    computeDimensions();
    initFonts();
    loadImages();

    fallingBalls.clear();
    ballTuple.clear();


    currentBall= new BallInfo(ball_radius, FPoint(0,0), COLOR_WHITE,BALLSTATE_STATIC);
    nextBall    = new BallInfo(ball_radius, FPoint(0,0), COLOR_WHITE,BALLSTATE_STATIC);

    initCursor();//COMPLETE

    initWell();

    warnedForQuit=false;

    //initGame();//to bypass gameMenu
    gameMenuInit();

    flashEffect = new Flash( 0.3f, &rcWell);


    #ifdef CONSOLE_LOG
        sprintf(glbLogMsg,"Ingame:Constructor end\n");
        mySDL_Log(glbLogMsg);
    #endif

}
//-------------------------------------------------------

void InGame::computeDimensions(SDL_Rect* displayArea)//DONE
{
    DEBUG_REC();
    #ifdef CONSOLE_LOG
        sprintf(glbLogMsg,"InGame: computeDimensions\n");
        mySDL_Log(glbLogMsg);
    #endif


    {
        int minDim = std::min (displayArea->w/(WELL_WIDTH+1),displayArea->h/(WELL_HEIGHT+1));

        ball_radius=minDim/2;
        collision_slack = 0.033*ball_radius;
    }

    recs[TEX_WELL_BACKGROUND]->w=2*WELL_WIDTH*ball_radius;
    recs[TEX_WELL_BACKGROUND]->h=2*WELL_HEIGHT*ball_radius;
    recs[TEX_WELL_BACKGROUND]->x=(displayArea->w-recs[TEX_WELL_BACKGROUND]->w)/2;
    recs[TEX_WELL_BACKGROUND]->y=(displayArea->h-recs[TEX_WELL_BACKGROUND]->h)/2;

    rcWell = *(recs[TEX_WELL_BACKGROUND]);

    recs[TEX_WELL_TOP]->h = 1.25*ball_radius;//reaches to top of well
    recs[TEX_WELL_TOP]->x = recs[TEX_WELL_BACKGROUND]->x;
    recs[TEX_WELL_TOP]->y = recs[TEX_WELL_BACKGROUND]->y-recs[TEX_WELL_TOP]->h;
    recs[TEX_WELL_TOP]->w = recs[TEX_WELL_BACKGROUND]->w;


    recs[TEX_WELL_BOTTOM]->x = recs[TEX_WELL_TOP]->x;
    recs[TEX_WELL_BOTTOM]->y = recs[TEX_WELL_BACKGROUND]->y + recs[TEX_WELL_BACKGROUND]->h;
    recs[TEX_WELL_BOTTOM]->w = recs[TEX_WELL_TOP]->w;
    recs[TEX_WELL_BOTTOM]->h = recs[TEX_WELL_TOP]->h;

    recs[TEX_WELL_LEFT]->w = ball_radius;
    recs[TEX_WELL_LEFT]->x = recs[TEX_WELL_BACKGROUND]->x - recs[TEX_WELL_LEFT]->w;
    recs[TEX_WELL_LEFT]->y = recs[TEX_WELL_BACKGROUND]->y;
    recs[TEX_WELL_LEFT]->h = recs[TEX_WELL_BACKGROUND]->h;//+recs[TEX_WELL_TOP]->h+recs[TEX_WELL_BOTTOM]->h;

    recs[TEX_WELL_RIGHT]->x = recs[TEX_WELL_BACKGROUND]->x+recs[TEX_WELL_BACKGROUND]->w;
    recs[TEX_WELL_RIGHT]->y = recs[TEX_WELL_LEFT]->y;
    recs[TEX_WELL_RIGHT]->w = recs[TEX_WELL_LEFT]->w;
    recs[TEX_WELL_RIGHT]->h = recs[TEX_WELL_LEFT]->h;

    recs[TEX_WELL_LCORNER]->x = recs[TEX_WELL_LEFT]->x;
    recs[TEX_WELL_LCORNER]->y = recs[TEX_WELL_TOP]->y;
    recs[TEX_WELL_LCORNER]->w = recs[TEX_WELL_LEFT]->w;
    recs[TEX_WELL_LCORNER]->h = recs[TEX_WELL_TOP]->h;

    recs[TEX_WELL_RCORNER]->x = recs[TEX_WELL_RIGHT]->x;
    recs[TEX_WELL_RCORNER]->y = recs[TEX_WELL_LCORNER]->y;
    recs[TEX_WELL_RCORNER]->w = recs[TEX_WELL_LCORNER]->w;
    recs[TEX_WELL_RCORNER]->h = recs[TEX_WELL_LCORNER]->h;


    rcBorder.x = recs[TEX_WELL_LEFT]->x;
    rcBorder.y = recs[TEX_WELL_TOP]->y;
    rcBorder.w = rcWell.w + 2*recs[TEX_WELL_LEFT]->w;
    rcBorder.h = rcWell.h+2*recs[TEX_WELL_TOP]->h;

    recs[TEX_WELL_PISTONS]->x = recs[TEX_WELL_BACKGROUND]->x;
    recs[TEX_WELL_PISTONS]->y = recs[TEX_WELL_BACKGROUND]->y;
    recs[TEX_WELL_PISTONS]->w = recs[TEX_WELL_BACKGROUND]->w;
    recs[TEX_WELL_PISTONS]->h = (piston_step==0)?0
                                    :getYCoord(piston_step-1)-ball_radius-recs[TEX_WELL_BACKGROUND]->y;

    recs[TEX_WELL_TOP_PLATE]->h = ball_radius*SQRT3;
    recs[TEX_WELL_TOP_PLATE]->x = recs[TEX_WELL_BACKGROUND]->x;
    recs[TEX_WELL_TOP_PLATE]->w = recs[TEX_WELL_BACKGROUND]->w;
    recs[TEX_WELL_TOP_PLATE]->y = (piston_step==0)?
                                    recs[TEX_WELL_BACKGROUND]->y-recs[TEX_WELL_TOP_PLATE]->h
                                    :getYCoord(piston_step-1)-ball_radius;

    topSurfaceY=recs[TEX_WELL_TOP_PLATE]->y + recs[TEX_WELL_TOP_PLATE]->h;

    recs[TEX_WELL_PISTON_BACKGROUND]->x = recs[TEX_WELL_BACKGROUND]->x;
    recs[TEX_WELL_PISTON_BACKGROUND]->y = recs[TEX_WELL_BACKGROUND]->y;
    recs[TEX_WELL_PISTON_BACKGROUND]->w = recs[TEX_WELL_BACKGROUND]->w;
    recs[TEX_WELL_PISTON_BACKGROUND]->h = topSurfaceY - recs[TEX_WELL_PISTON_BACKGROUND]->y;

    recs[TEX_MACHINE]->x = recs[TEX_WELL_BACKGROUND]->x;
    recs[TEX_MACHINE]->y = recs[TEX_WELL_BOTTOM]->y-4*ball_radius;
    recs[TEX_MACHINE]->w = recs[TEX_WELL_BACKGROUND]->w;
    recs[TEX_MACHINE]->h = 4*ball_radius;

    recs[TEX_CANNON]->w = 2*ball_radius;
    recs[TEX_CANNON]->h = 6*ball_radius;
    recs[TEX_CANNON]->x = recs[TEX_WELL_BACKGROUND]->x + ( recs[TEX_WELL_BACKGROUND]->w -recs[TEX_CANNON]->w ) / 2 ;
    recs[TEX_CANNON]->y = recs[TEX_MACHINE]->y - recs[TEX_CANNON]->h/6;


    recs[TEX_WELL_ENDLINE]->x = recs[TEX_WELL_BACKGROUND]->x;
    recs[TEX_WELL_ENDLINE]->y = getYCoord(WELL_HEIGHT)-ball_radius/2;
    recs[TEX_WELL_ENDLINE]->w = recs[TEX_WELL_BACKGROUND]->w;
    recs[TEX_WELL_ENDLINE]->h = 2;//std::max(2.,ball_radius/8.);


    launchPoint.x=recs[TEX_CANNON]->x +recs[TEX_CANNON]->w/2;
    launchPoint.y=recs[TEX_CANNON]->y +recs[TEX_CANNON]->h/2;

    summonPoint.x = rcWell.x+ball_radius;
    summonPoint.y = rcWell.y +rcWell.h - ball_radius*0.825;

    for (int i=0;i<NUM_BALLCOLORS+NUM_OBSTACLES;i++)
    {
        recs[i]->x = 0;
        recs[i]->y = 0;
        recs[i]->w = 2*ball_radius;
        recs[i]->h = 2*ball_radius;
    }

    recs[TEX_BALL_INDEX]->x = 0;
    recs[TEX_BALL_INDEX]->y = 0;
    recs[TEX_BALL_INDEX]->w = ball_radius/2;
    recs[TEX_BALL_INDEX]->h = ball_radius/2;

    recs[TEX_TIMESTOP]->x = 0;
    recs[TEX_TIMESTOP]->y = 0;
    recs[TEX_TIMESTOP]->w = 1.65*ball_radius;
    recs[TEX_TIMESTOP]->h = 1.65*ball_radius;

    if ( (playState!=MODE_GAME_MENU) && (playState!=MODE_LEVEL_SELECT) )
    {
        if (gameSettings->useBallIndex)
            computeBallIndex();

        SparksEffect::setWellRectangle(rcWell);

        BALL_VELOCITY = 2.0*rcWell.h;

        launchLimit = launchPoint.y;//recs[TEX_WELL_ENDLINE]->y;

        if (trailsEffect!=NULL)
        {
            delete trailsEffect;
            trailsEffect=NULL;
        }
        trailsEffect=new TrailsEffect(displayArea->w,displayArea->h);
    }
}
//-------------------------------------------------------
void InGame::initFonts()
{
    DEBUG_REC();

    if (largeMessageFont!=NULL)  TTF_CloseFont( largeMessageFont );
    if (mediumMessageFont!=NULL) TTF_CloseFont( mediumMessageFont );
    if (smallMessageFont!=NULL) TTF_CloseFont( smallMessageFont );

    largeMessageFont=TTF_OpenFont( "media/gamefont.otf", ball_radius*2 );
    mediumMessageFont=TTF_OpenFont( "media/gamefont.otf", ball_radius*1.1 );
    smallMessageFont=TTF_OpenFont( "media/gamefont.otf", ball_radius*0.85 );
}
//-------------------------------------------------------
void InGame::initGame(int action, int level)
{
    DEBUG_REC();
    #ifdef CONSOLE_LOG
        sprintf(glbLogMsg,"entering InitGame\n");
        mySDL_Log(glbLogMsg);
    #endif
#ifdef PLATFORM_ANDROID
    meanShotTime=1.;
#endif
//--------------MOVED FROM CONSTRUCTOR-----------------------------
    piston_step =0;//needed by compute dimensions
    initial_step=piston_step;

    computeDimensions();

    loadPlayMusic("media/ingame.ogg");
//---------------------------------------------------------------

    if ( action== ACTION_PLAYLEVEL)
        gameLevel=level;
    else
        gameLevel=0;

    score=playerStats->getMaxScore(gameLevel);

    initWell();

    initRound( action ==ACTION_LOAD_SAVED );

    setVelocity();

    setDurations();

    #ifdef CONSOLE_LOG
        sprintf(glbLogMsg,"done InitGame\n");
        mySDL_Log(glbLogMsg);
    #endif
}
//-------------------------------------------------------
void InGame::endLevel(bool hasWon)
{
    DEBUG_REC();
    bool scoreRecord = false;
    bool timeRecord = false;
    bool shotRecord = false;
    int unlockedFeature=0;

    int duration=roundTimer.GetTicks();
    if (hasWon)
    {
        if ( duration < playerStats->getMinTime(gameLevel) )
            timeRecord=true;
        if ( playerStats->getLevelShots() < playerStats->getMinShots(gameLevel))
            shotRecord=true;
        if ( roundScore  >  playerStats->getRoundScore(gameLevel))
            scoreRecord=true;

        unlockedFeature=playerStats->addVictory(gameLevel,roundScore,duration,(ballSummoned?ballToSummon:UNL_NONE),timeStopUsed,match4,match5,match6,numInitialBalls,startSkin);
    }
    else
        playerStats->addPlayTime(duration);


    pauseTimers();

    //unlockedFeature = 5+(1<<11);//testing

    endGame = new EndGame(rcWell, gameLevel, hasWon,scoreRecord,timeRecord,shotRecord,unlockedFeature);

    if (hasWon)
    {
        gameLevel++;
    #ifdef DEMO_GAME
        gameLevel=std::min(gameLevel,MAX_DEMO_LEVEL);
    #else
        if (!isLegitimateCopy)
            gameLevel=std::min(gameLevel,MAX_DEMO_LEVEL);
    #endif // DEMO_GAME

        score+=roundScore;

        playerStats->addAttempt(gameLevel);//to record max level

    }
    playState=MODE_ENDGAME;

}
//-------------------------------------------------------
void InGame::setDurations()
{
    DEBUG_REC();
    switch (gameSettings->pressure){
        case PRESSURE_NONE:   pistonDuration=1000000000;shotDuration=1000000000;break;
        case PRESSURE_MODERATE:   pistonDuration=18013;shotDuration=4000;break;
        case PRESSURE_HIGH:   pistonDuration=8999;shotDuration=2000;break;
        case PRESSURE_HIGHEST:   pistonDuration=5999;shotDuration=1300;break;
    };

}
//-------------------------------------------------------
void InGame::setVelocity()
{
    DEBUG_REC();
    switch (gameSettings->pressure){
        case PRESSURE_NONE:     velocityMultiplier=1.1;break;
        case PRESSURE_MODERATE: velocityMultiplier=1.2;break;
        case PRESSURE_HIGH:     velocityMultiplier=1.5;break;
        case PRESSURE_HIGHEST:  velocityMultiplier=1.8;break;
    };

}
//-------------------------------------------------------
void InGame::initRound(bool loadSavedState)
{
    DEBUG_REC();
    #ifdef CONSOLE_LOG
        sprintf(glbLogMsg,"entering InitRound: level=%d\n",gameLevel);
        mySDL_Log(glbLogMsg);
    #endif


    playerStats->addAttempt(gameLevel);

    angle=0;
    angleDiff=1;

    roundScore=0;
    match4=0;
    match5=0;
    match6=0;

    editorSeq=0;

    warnedForQuit=false;
    powerBallGenerated=false;
    timeStopActive=false;
    timeStopUsed=false;
    ballToSummon=UNL_NONE;
    ballSummoned=false;

    startSkin = gameSettings->skin;



#ifdef PLATFORM_ANDROID
    touchHappened=false;
    justSwitched =false;
    lastShotWasAuto = false;
#endif
    emptyWell();
    resetTopPlate();

    initBalls();

    bool oldStateLoaded=false;
    if (loadSavedState)
    {
        oldStateLoaded=loadGameState();
    #ifdef DEMO_GAME
        if (gameLevel>MAX_DEMO_LEVEL)
        {
            oldStateLoaded=false;
            gameLevel=0;
            emptyWell();
        }
    #else
        if ((!isLegitimateCopy)&&(gameLevel>MAX_DEMO_LEVEL))
        {
            oldStateLoaded=false;
            gameLevel=0;
            emptyWell();
        }
    #endif

    }

    if (!oldStateLoaded)
    {
    #ifdef DEMO_GAME
        gameLevel=std::min(gameLevel,MAX_DEMO_LEVEL);
    #else
        if (!isLegitimateCopy)
            gameLevel=std::min(gameLevel,MAX_DEMO_LEVEL);
    #endif // DEMO_GAME
        readLevel(gameLevel);
        initTimers();
        pauseTimers();//to simplify countdown screen end
    }

    if (gameSettings->useBallIndex)
        computeBallIndex();

    invTexStatic=true;
    invTexShining=true;
    invTexWellHardware=true;
    invTexLauncher = true;

    isEffectActive=false;

    setSummonIndicator();

    updateStaticBallImage();//OPTIMIZATION
    updateShineEffectImage();
    if (gameSettings->useBallIndex)
        updateBallIndexImage();
    updateLauncherImage();


    updateLevelMessage();

    createLegendMessages();

   if (gameSettings->showBest)
        createBestMessages();

   if (gameSettings->showRoundScore)
            updateRoundScoreMessage();
    //else
      //  updateScoreMessage();

    if (gameSettings->showRoundTime)
        updateRoundTimeMessage(true);
    if (gameSettings->showRoundShots)
        updateRoundShotsMessage();


    updateWellHardwareImage();


    if (playState!=MODE_EDITOR)
    {
        playState = MODE_PREGAME;

        startCountDownTimer();
        setGameMessage("Get Ready");
    }
    if (timeStopActive)
        activateTimeStop();


    #ifdef CONSOLE_LOG
        sprintf(glbLogMsg,"end InitRound\n");
        mySDL_Log(glbLogMsg);
    #endif

}
//---------------------------------------------
void InGame::setSummonIndicator()
{
    if (summonEffect!=NULL)
    {
        delete summonEffect;
        summonEffect=NULL;
    }

    if (
        ( !playerStats->getAvailableSummonsVector(gameLevel) )
        ||
        (ballSummoned && timeStopUsed && (!timeStopActive))
        )
    {
        texSummon = glbTexManager->getTexturePtr( handles[TEX_TIMESTOP]);
        summonAlpha = 100;//50;
    }
    else
    {
        if ((ballToSummon!=UNL_NONE)&&(!ballSummoned))
        {
            int summon_radius=recs[TEX_TIMESTOP]->w/2;

            switch (ballToSummon)
            {
                case UNL_BOMB:          texSummon=glbTexManager->getTexturePtr( handles[COLOR_BOMB]);
                                        summonEffect= new FuseEffect( summonPoint, FPoint(27.,-32).GetNormal()*summon_radius*0.9,summon_radius/4,FPoint(0,BALL_VELOCITY) );
                                        break;

                case UNL_COLORSHIFTER:  texSummon=glbTexManager->getTexturePtr( handles[COLOR_SHIFTER]);
                                        summonEffect= new PixiesEffect( summonPoint, COLOR_SHIFTER, summon_radius/4);
                                        break;

                case UNL_PAINTER:       texSummon=glbTexManager->getTexturePtr( handles[COLOR_PAINTER]);
                                        summonEffect= new PixiesEffect( summonPoint, COLOR_PAINTER, summon_radius/4);
                                        break;

                case UNL_LEVELER:       texSummon=glbTexManager->getTexturePtr( handles[COLOR_LEVELER]);
                                        summonEffect= new PixiesEffect( summonPoint, COLOR_LEVELER, summon_radius/4);
                                        break;

                case UNL_CHROME:        texSummon=glbTexManager->getTexturePtr( handles[COLOR_CHROME]);
                                        summonEffect= new PixiesEffect( summonPoint, COLOR_CHROME, summon_radius/4);
                                        break;

            }
            summonAlpha=255;//200
        }
        else
        if (timeStopActive)
        {
            texSummon = glbTexManager->getTexturePtr( handles[TEX_TIMESTOP]);
            summonAlpha = 255;//200
        }
        else
        {
            texSummon = glbTexManager->getTexturePtr( handles[TEX_TIMESTOP]);
            summonAlpha = 170;//120
        }

    }
    invTexLauncher=true;
}
//---------------------------------------------
void InGame::updateLevelMessage()
{
    DEBUG_REC();

    char msg[20];

    sprintf(msg,"Level:%5d",gameLevel+1);
    texLevel->fromText(msg,glbTextColor,mediumMessageFont);//,color);
}
//---------------------------------------------
/*
void InGame::updateScoreMessage()
{
    DEBUG_REC();

    char msg[20];

    sprintf(msg,"Score:%6d",score+roundScore);
    texScore->fromText(msg,glbTextColor,mediumMessageFont);
}*/

//----------------------------------------------
void InGame::createLegendMessages()
{
    DEBUG_REC();

    texRoundScore[STAT_LEGEND]->fromText("score",gameSettings->colors[COLOR_INGAME_BGSTAT],smallMessageFont);

    texRoundTime[STAT_LEGEND]->fromText("time",gameSettings->colors[COLOR_INGAME_BORDERSTAT],smallMessageFont);

    texRoundShots[STAT_LEGEND]->fromText("shots",gameSettings->colors[COLOR_INGAME_BORDERSTAT],smallMessageFont);
}

//----------------------------------------------
void InGame::createBestMessages()
{
    DEBUG_REC();

    char msg[20]="---";


    bool passed=playerStats->hasBeaten(gameLevel);

    if (passed)
        sprintf(msg,"%4d",playerStats->getRoundScore(gameLevel));

    texRoundScore[STAT_BEST]->fromText(msg,gameSettings->colors[COLOR_INGAME_BGBEST],mediumMessageFont);


    if (passed)
    {
        int mintime=playerStats->getMinTime(gameLevel);
        sprintf(msg,"%d . %03d",mintime/1000,mintime%1000);
    }

    texRoundTime[STAT_BEST]->fromText(msg,gameSettings->colors[COLOR_INGAME_BORDERBEST],smallMessageFont);


    if (passed)
        sprintf(msg,"%d",playerStats->getMinShots(gameLevel));

    texRoundShots[STAT_BEST]->fromText(msg,gameSettings->colors[COLOR_INGAME_BORDERBEST],smallMessageFont);
}
//-----------------------------------------------------------
void InGame::updateRoundScoreMessage()
{
    DEBUG_REC();

    char msg[20];

    sprintf(msg,"%4d",roundScore);
    texRoundScore[STAT_CURRENT]->fromText(msg,gameSettings->colors[COLOR_INGAME_BGSTAT],mediumMessageFont);

}
//-----------------------------------------------------------
void InGame::updateRoundTimeMessage(bool forcedUpdate)
{
    DEBUG_REC();

    char msg[20];

    static int prevTime=-1;
    int ticks=roundTimer.GetTicks()/100;

    if (prevTime!=ticks || forcedUpdate)
    {
        sprintf(msg,"%d . %1d",ticks/10,ticks%10);
        texRoundTime[STAT_CURRENT]->fromText(msg,gameSettings->colors[COLOR_INGAME_BORDERSTAT],smallMessageFont);
        prevTime=ticks;
    }

}
//-----------------------------------------------------------
void InGame::updateRoundShotsMessage()
{
    DEBUG_REC();

    char msg[20]="";

    sprintf(msg,"%d",playerStats->getLevelShots());

    texRoundShots[STAT_CURRENT]->fromText(msg,gameSettings->colors[COLOR_INGAME_BORDERSTAT],smallMessageFont);

}
//----------------------------------------------
void InGame::setGameMessage(std::string msg)
{
    DEBUG_REC();

    texMessage->fromText(msg,gameSettings->colors[COLOR_INGAME_MESSAGE],largeMessageFont);
}
//----------------------------------------------
void InGame::updateCountDownMessage()
{
    DEBUG_REC();

    static int lastNum=4;

    int num=(int)(4.-countDownTimer.GetTicks()/1000.);

    if (lastNum != num)
    {
        char msg[20];

       sprintf(msg,"%d", num);
       texCountDown->fromText(msg,gameSettings->colors[COLOR_INGAME_MESSAGE],largeMessageFont);
       lastNum=num;
    }
}

//--------------------------------------------------

void InGame::initWell()
{
    DEBUG_REC();

    for( int i=0; i < getWellMatrixHeight(); i++)
        for ( int j=0; j < WELL_WIDTH; j++)
            well[i][j]=NULL;
}

//--------------------------------------------------
void InGame::deleteFallingBalls()
{
    DEBUG_REC();

    std::list<BallInfo*>::iterator li;
    for( li= fallingBalls.begin(); li != fallingBalls.end();li++)
    {
        if (*li ==NULL ) continue;

         delete *li;
        (*li)=NULL;
    }
    fallingBalls.clear();
}
//--------------------------------------------------
void InGame::emptyWell()
{
    DEBUG_REC();

    for( int i=0; i < getWellMatrixHeight(); i++)
        for ( int j=0; j < WELL_WIDTH; j++)
            if (well[i][j]!=NULL)
            {
                delete well[i][j];
                well[i][j]=NULL;
            }
    for (int i=0;i<NUM_BALLCOLORS;i++)
        colorStats[i]=0;

    numStaticBalls=0;
}

//--------------------------------------------------

void InGame::initBalls(int currentColor)//DONE
{
    DEBUG_REC();

    if (currentColor!=-1)//not initial color assignment
    {
        BallInfo* temp;

        temp=currentBall;
        currentBall=nextBall;
        nextBall=temp;

        temp=NULL;
    }

    currentBall->setPosition(launchPoint);
    currentBall->setVelocity(FPoint(0,0));
    currentBall->setState(BALLSTATE_READY);

    if (playState==MODE_PLAY)//in editor mode user chooses color
    {
        if (currentColor==-1)
        {
            currentBall->setColor(chooseNextBallColor(false));
        }
        //else  currentBall->setColor(currentColor);//no need, already swapped
    }

    currentBall->setAlpha(255);

    nextBall->setPosition(launchPoint +FPoint( recs[TEX_WELL_BACKGROUND]->w/4.,0.));

    nextBall->setVelocity(FPoint(0,0));

    nextBall->setState(BALLSTATE_STATIC);
    nextBall->setColor(chooseNextBallColor(true));
    nextBall->setAlpha(255);

    if (gameSettings->useBallIndex)
        computeBallIndex();
}
//------------------------------------------------------
int InGame::findLowestBall(GridPoint& gp)
{
    DEBUG_REC();

    int maxHeight=0;
    float minDist=5.;

    bool found=false;

    for (int row = getWellMatrixHeight()-1-piston_step; (row>=0)&&(!found);row--)
    {
        float rowBalls=maxBalls(row);
        for (int col=0;col<rowBalls;col++)
        {

            if (well[row][col]!=NULL)
            {
                found=true;
                maxHeight=row;
                float dist= std::abs(col*1.f-rowBalls/2);
                if (dist<minDist)
                {
                    gp.col=col;
                    gp.row=row;
                }
            }
        }
    }
    return maxHeight;
}
//-----------------------------------------------------
void InGame::switchColors()
{
    DEBUG_REC();
    //int temp=currentBall->getColor();
    //currentBall->setColor( nextBall->getColor() );
    //nextBall->setColor(temp);
    soundEffect(SFX_SWAP);
    BallInfo* temp=currentBall;//swapping balls, so effects can continue existing

    currentBall=nextBall;
    nextBall=temp;

    nextBall->setVelocity(currentBall->getVelocity() );
    nextBall->setPosition(currentBall->getPosition() );
    nextBall->setState(BALLSTATE_STATIC);


    currentBall->setVelocity(FPoint(0.,0.));
    currentBall->setPosition(launchPoint);
    currentBall->setState(BALLSTATE_READY);

    temp=NULL;

#ifdef PLATFORM_ANDROID
    touchHappened=false;
    justSwitched=false;
    justSwitched=true;
#endif // PLATFORM_ANDROID
}


int InGame::chooseNextBallColor(bool checkCurrent)//DONE
{
    DEBUG_REC();

    int i,n,color;
    if ((ballToSummon==UNL_NONE) || (ballSummoned))
    {
        float stats[NUM_BALLCOLORS];
        float pR;//probability next color will be true Random
        float pB=0.f;//probability next color will be COLOR_BOMB
        float pC=0.f;//probability next color will be COLOR_CHROME
        float pS=0.f;//probability next color will be COLOR_SHIFTER
        float pL=0.f;//probability next color will be COLOR_LEVELER
        float pP=0.f;//probability next color will be COLOR_PAINTER

        float sum=0;


        switch (gameSettings->randomness){
            case RANDOMNESS_LOW:    pR=0.1;break;
            case RANDOMNESS_MODERATE: pR=0.3;break;
            case RANDOMNESS_HIGH:   pR=0.6;break;
            case RANDOMNESS_HIGHEST:   pR=1.;break;

        };

        pB=0.04 +(gameSettings->matchNumber-2)*0.02;

        pS=pB;

        switch (gameSettings->pressure){
            case PRESSURE_NONE:    pB/=2.; break;
            case PRESSURE_HIGH:   pB*=1.333;break;
            case PRESSURE_HIGHEST:   pB*=1.666;break;
        };
    //========================================================
        if (!powerBallGenerated)
        {
            GridPoint cB(0,0);

            float margin=WELL_HEIGHT-findLowestBall(cB)-piston_step;
            float maxMargin=WELL_HEIGHT-initial_step;
            float invMarginRatio= 1.f-margin/maxMargin;

            float baseP=invMarginRatio*invMarginRatio*invMarginRatio;
            pP=baseP*invMarginRatio/8;

            if (margin>=3) pC=0.;
            else
            {
                float middle=maxBalls(cB.row)/2.f;

                pC = baseP;
                pC*= pC;
                pC*= pC;
                pC*=std::max( 0.3f, 1.f - std::abs(cB.col-middle)/middle );//was 0.5f
            }
            pL =(pP/4.f + pC)/2.f;



    #ifdef CONSOLE_LOG
            sprintf(glbLogMsg," CHROME PROBABILITY %f margin=%f\n",pC,margin);
            mySDL_Log(glbLogMsg);
            sprintf(glbLogMsg," LEVELER PROBABILITY %f margin=%f\n",pL,margin);
            mySDL_Log(glbLogMsg);
            sprintf(glbLogMsg," PAINTER PROBABILITY %f margin=%f\n",pP,margin);
            mySDL_Log(glbLogMsg);
    #endif
        }
        else
           pC=pL=pP=0.;

    //==============================================================

    //=================DEBUG - TESTING  VALUES===================
    //    pB=0.1;
    //    pS=0.1;
    //    pP=0.1;
    //    pL=0.1;
    //    pC=0.1;
    //============================================================
        //ADJUST SPECIAL BALL PROBABITILY, SO THAT IT DOES NOT EXCEED 1


        float divisor=10.;// AS they unlock features, next ones become more probable

        if (!playerStats->isSummonUnlocked(UNL_BOMB,gameLevel))            {pB/=divisor; divisor*=2;}
        if (!playerStats->isSummonUnlocked(UNL_COLORSHIFTER,gameLevel))    {pS/=divisor; divisor*=2;}
        if (!playerStats->isSummonUnlocked(UNL_PAINTER,gameLevel))         {pP/=divisor; divisor*=2;}
        if (!playerStats->isSummonUnlocked(UNL_LEVELER,gameLevel))         {pL/=divisor; divisor*=2;}
        if (!playerStats->isSummonUnlocked(UNL_CHROME,gameLevel))          {pC/=divisor; divisor*=2;}

        if (!gameSettings->useBombs)        pB=0.;
        if (!gameSettings->useShifters)     pS=0.;
        if (!gameSettings->usePainters)     pP=0.;
        if (!gameSettings->useLevelers)     pL=0.;
        if (!gameSettings->useChromeBalls)  pC=0.;

        if (pB==pS)//both unlocked or locked
            pB=pS=pS/2.;//halving the common special ball probability


    //=================DEBUG - TESTING  VALUES===================
        //pB=0.1;
        //pS=0.1;
        //pP=0.2;
        //pL=0.1;
        //pC=0.2;
    //============================================================
        //ADJUST SPECIAL BALL PROBABITILY, SO THAT IT DOES NOT EXCEED 1



        for (i=0;i<NUM_BALLCOLORS-NUM_SPECIALBALLS;i++)//WAS 1
        {
            stats[i]=colorStats[i];
            sum+=stats[i];
        }

        if (checkCurrent)//include the current ball in the computation
        {
/*            if (currentBall->getColor() < (NUM_BALLCOLORS-NUM_SPECIALBALLS) )
            {
                sum++;
                stats[currentBall->getColor()]++;
            }
            else*/
            if ( (currentBall->getColor() >= COLOR_BOMB) )
            {
                pC=pB=pS=pL=pP=0.f;  //not two in a row, never
            }

        }

        if (sum==0)
        {
    #ifdef CONSOLE_LOG
            sprintf(glbLogMsg,"ChooseNextColor, well is empty\n");
            mySDL_Log(glbLogMsg);
    #endif
            color = genRand10000()%(NUM_BALLCOLORS-NUM_SPECIALBALLS);//a random color//WAS 1
        }
        else
        {
            float weightedSum=0.;

    #ifdef CONSOLE_LOG
            sprintf(glbLogMsg,"ChooseNextColor, Real stats: \n");
            mySDL_Log(glbLogMsg);
            for (i=0;i<NUM_BALLCOLORS-NUM_SPECIALBALLS;i++)//WAS-1
            {
                sprintf(glbLogMsg," %.2f \n",100.f*stats[i]/sum);
                mySDL_Log(glbLogMsg);
            }
    #endif

            float pSpecial=pB+pC+pS+pL+pP;

            for (i=0;i<NUM_BALLCOLORS-NUM_SPECIALBALLS;i++)
            {
                stats[i]=10000.*( (1.-(pSpecial))*(pR/(NUM_BALLCOLORS-NUM_SPECIALBALLS) + (1.-pR)*stats[i]/sum ));
                weightedSum+=stats[i];
            }

            stats[COLOR_BOMB]=10000.*pB;
            stats[COLOR_SHIFTER]=10000.*pS;
            stats[COLOR_LEVELER]=10000.*pL;
            stats[COLOR_PAINTER]=10000.*pP;
            stats[COLOR_CHROME]=10000.*pC;

            weightedSum+=stats[COLOR_BOMB];
            weightedSum+=stats[COLOR_SHIFTER];
            weightedSum+=stats[COLOR_LEVELER];
            weightedSum+=stats[COLOR_PAINTER];
            weightedSum+=stats[COLOR_CHROME];

    #ifdef CONSOLE_LOG
            sprintf(glbLogMsg,"ChooseNextColor, Final probabilities: \n");
            mySDL_Log(glbLogMsg);
            for (i=0;i<NUM_BALLCOLORS;i++)
            {
                sprintf(glbLogMsg," %.2f \n",100*stats[i]/weightedSum);
                mySDL_Log(glbLogMsg);
            }
    #endif

            n = genRand10000();

    #ifdef CONSOLE_LOG
            sprintf(glbLogMsg,"random was %d , total is %d\n",n,(int)weightedSum);
            mySDL_Log(glbLogMsg);
    #endif

            sum=0.;

            color=NUM_BALLCOLORS-NUM_SPECIALBALLS;

            for ( i=0; i< NUM_BALLCOLORS; i++)
            {
                sum += stats[i];
                if ( (stats[i]>0) && (n <= sum) )
                {
                    color=i;
                    break;
                }
            }
        }
    }//if !summon
    else
    {
            switch (ballToSummon)
            {
                case UNL_BOMB:          color=COLOR_BOMB;break;
                case UNL_COLORSHIFTER:  color=COLOR_SHIFTER;break;
                case UNL_PAINTER:       color=COLOR_PAINTER;break;
                case UNL_LEVELER:       color=COLOR_LEVELER;break;
                case UNL_CHROME:        color=COLOR_CHROME;break;
            }
            ballSummoned=true;
            setSummonIndicator();
            flashEffect->shine(0.3,&rcWell);
    }

    if (color > COLOR_SHIFTER)//leveler painter chrome
        powerBallGenerated=true;

#ifdef CONSOLE_LOG
    sprintf(glbLogMsg,"color is %d\n",color);
    mySDL_Log(glbLogMsg);
#endif
    return color;
}

int InGame::generateRandomLevel(int levelmap[MAX_WELL_HEIGHT][MAX_WELL_WIDTH], RandomGenerator& levelGen,   int MAXRAND)
{

    bool genRandomColors=(levelGen.generateNext() < MAXRAND/2);
    bool genUsePeriod=(levelGen.generateNext() < MAXRAND/2);

    //int numLines=(WELL_HEIGHT/2 -1)+levelGen.generateNext()%(WELL_HEIGHT/2);
    //int ballsPerLine = WELL_WIDTH*0.75 + levelGen.generateNext()%(WELL_WIDTH/4);

    int numLines=(WELL_HEIGHT/3 -1)+levelGen.generateNext()%(WELL_HEIGHT/2);
    int ballsPerLine = WELL_WIDTH*0.75 + levelGen.generateNext()%(WELL_WIDTH/4);


    int startColor,period,linesPerPeriod;

    startColor=levelGen.generateNext()%(NUM_BALLCOLORS-NUM_SPECIALBALLS);//(level^modif)%(NUM_BALLCOLORS-NUM_SPECIALBALLS);
    period=3+levelGen.generateNext()%(NUM_BALLCOLORS-NUM_SPECIALBALLS-3);
    linesPerPeriod= 1+levelGen.generateNext()%(numLines/2);

    //----------------------------------------------

    int c=0;
    int l=0;

    for (int row=0; row < numLines; row++)
    {
        int b=0;

        while (b < ballsPerLine-2)
        {
            b=0;

            for(int col=0; col<maxBalls(row); col++)
            {
                levelmap[row][col]=0;

                if ( levelGen.generateNext() < (1.*MAXRAND*ballsPerLine)/maxBalls(row))
                {
                    if (genRandomColors)
                        if (genUsePeriod)
                            levelmap[row][col]=1+(startColor+levelGen.generateNext()%period)%(NUM_BALLCOLORS-NUM_SPECIALBALLS);
                        else
                            levelmap[row][col]=1+levelGen.generateNext()%(NUM_BALLCOLORS-NUM_SPECIALBALLS-1);
                    else
                    {
                        levelmap[row][col]=(startColor+c)%(NUM_BALLCOLORS-NUM_SPECIALBALLS);
                        c= (c+1)%period;
                    }
                    b++;
                }
            }
        }
        l++;
        if (l==linesPerPeriod)
        {
            period=3+levelGen.generateNext()%(NUM_BALLCOLORS-NUM_SPECIALBALLS-3);//-5
            linesPerPeriod= 1+levelGen.generateNext()%(WELL_WIDTH/3);
            startColor=levelGen.generateNext()%(NUM_BALLCOLORS-NUM_SPECIALBALLS);
            l=0;
        }
    }
    return numLines;
}
//------------------------------------------------
int InGame::generatePlasmaLevel(int levelmap[MAX_WELL_HEIGHT][MAX_WELL_WIDTH], RandomGenerator& levelGen)
{
        int palette[128];
        for (int i=0;i<128;i++)
        {

           palette[i]=1+((1+levelGen.generateNext()%7)*i/(NUM_BALLCOLORS-NUM_SPECIALBALLS))%(NUM_BALLCOLORS-NUM_SPECIALBALLS);//need 0 to be empty space. all others map to ball color+1
        }

        int numLines=(WELL_HEIGHT/3)+levelGen.generateNext()%(WELL_HEIGHT/2);

        int numFuncs=1+levelGen.generateNext()%5;//2-4 functions

        float bcFreq=(PI*(levelGen.generateNext()%(2*WELL_WIDTH)))/WELL_WIDTH;//bottom contour frequency
        float bcPhase=(PI*(levelGen.generateNext()%WELL_WIDTH))/WELL_WIDTH;//bottom contour phase shift

        float maxAmp=0.125*(1.+(1.*(levelGen.generateNext()%WELL_WIDTH))/WELL_WIDTH);
        float bcAmp=(maxAmp*numLines*(levelGen.generateNext()%WELL_WIDTH))/WELL_WIDTH;

        FPoint centers[5];

        int f,index;

        int colDepth[MAX_WELL_WIDTH];
        for (int i=0;i<WELL_WIDTH;i++)
            colDepth[i]=numLines - bcAmp  + bcAmp*mySine(bcFreq*(0.5*i/PI)+bcPhase);

        for (int i=0;i<numFuncs;i++)
        {
            centers[i].x=levelGen.generateNext()%WELL_WIDTH;
            centers[i].y=levelGen.generateNext()%WELL_HEIGHT;
        }
        int maxLine=numLines-1;
        bool used;

        for (int row=0;row<numLines;row++)
        {
            used=false;
            int rowLength = maxBalls(row);
            for ( int col = 0; col < rowLength; col++ )
            {
                if (row>colDepth[col]) continue;
                FPoint p(col,row);
                 f= 64 +63*mySine( 2*(p-centers[0]).GetLength()/WELL_WIDTH );

                if (numFuncs>1)
                  f+= 64 +63*mySine( 2*(p-centers[1]).GetLength()/WELL_WIDTH );
                if (numFuncs>2)
                  f+= 64 +63*mySine( (p-centers[2]).GetLength()/WELL_WIDTH );
                if (numFuncs>3)
                  f+= 64 +63*mySine( 4*(p-centers[3]).GetLength()/WELL_WIDTH );
                if (numFuncs>4)
                  f+=   64 + 63 * mySine( row/(centers[4].y+1+(centers[4].y/2)*myCosine(2*col/WELL_WIDTH)) ) * myCosine( col/((centers[4].x+1)+(centers[4].x/2)*mySine(2*row/WELL_HEIGHT))) ;

                index = f/numFuncs;

                levelmap[row][col]=palette[index];
                used=true;
            }
            if (!used)
            {
                maxLine=row;
                break;
            }

        }
        return maxLine+1;//numLines;
}
//------------------------------------------------
int InGame::generateBoxMapLevel(int levelmap[MAX_WELL_HEIGHT][MAX_WELL_WIDTH], RandomGenerator& levelGen)
{
    int numBoxes=WELL_HEIGHT;// + (levelGen.generateNext()%WELL_HEIGHT);

    int numLines=(WELL_HEIGHT/3)+levelGen.generateNext()%(WELL_HEIGHT/2);// was /3-1

    int x,y,w,h,c;
    for (int i=0;i<numBoxes;i++)
    {
        if (i==0)
        {
            x=levelGen.generateNext()%(WELL_WIDTH/2);
            y=0;
            w=2+levelGen.generateNext()%(WELL_WIDTH-x-1);
            h=numLines;

            c=1+levelGen.generateNext()%(NUM_BALLCOLORS-NUM_SPECIALBALLS);
        }
        else
        {
            x=levelGen.generateNext()%(WELL_WIDTH-2);
            y=levelGen.generateNext()%(numLines-2);
            w=1+levelGen.generateNext()%(WELL_WIDTH-x);
            h=1+levelGen.generateNext()%(numLines-y);
            c=levelGen.generateNext()%(NUM_BALLCOLORS-NUM_SPECIALBALLS);
        }

        for (int row=y;row< y+h ;row++)
        {
            int lastCol = std::min(maxBalls(row),x+w);
            if (x==maxBalls(row)) continue;
            for ( int col = x; col < lastCol; col++ )
            {
                levelmap[row][col]=1+(levelmap[row][col]+c)%(NUM_BALLCOLORS-NUM_SPECIALBALLS);
            }

        }
    }
    return numLines;
}
//------------------------------------------------
void InGame::generateLevelBlocks(int levelmap[MAX_WELL_HEIGHT][MAX_WELL_WIDTH], RandomGenerator& levelGen, int numLines)
{
    int numBlocks= std::min(3+levelGen.generateNext()%4, numLines/3);
    int rowDiff=numLines/numBlocks;
    int row=rowDiff-1;

    for (int i=0;i<numBlocks;i++)
    {
        int blockLength=2+(levelGen.generateNext()%row)% WELL_WIDTH/2;
        int first = levelGen.generateNext()%(maxBalls(row)-blockLength);

        for( int c=first; c< first+blockLength; c++)
             levelmap[row][c]=COLOR_BLOCK+1;
        row+=rowDiff;
    }
}
//------------------------------------------------
void InGame::generateLevel(int level,bool selectorMode)
{
    DEBUG_REC();

    emptyWell();
    resetTopPlate();

    int levelmap[MAX_WELL_HEIGHT][MAX_WELL_WIDTH];

    int row,col;

    for (row=0; row<WELL_HEIGHT; row++)
        for(col=0; col<WELL_WIDTH; col++)
            levelmap[row][col]=0;


    //----------------------------------------------
    int MAXRAND=1000000;
    RandomGenerator levelGen(0,MAXRAND,level);

    int p =levelGen.generateNext()%100;
    int method=p/45;

    int numLines;

    switch (method)
    {
        case 0:numLines=generatePlasmaLevel(levelmap,levelGen);break;
        case 1:numLines=generateBoxMapLevel(levelmap,levelGen);break;
        case 2:numLines=generateRandomLevel(levelmap,levelGen,MAXRAND);break;
    }

    if (levelGen.generateNext()&1)
        generateLevelBlocks(levelmap,levelGen,numLines);

//--------------creating balls from color map---------------------

    for ( row = 0 ; row < numLines; row++)
    {
        int rowLength = maxBalls(row);

        for ( col = 0; col < rowLength; col++ )
        {
            if (levelmap[row][col]>0)
            {
                createBall( GridPoint(row,col), levelmap[row][col] - 1, !selectorMode) ;
                numStaticBalls++;

                colorStats[ levelmap[row][col] - 1 ]++;
            }
        }
    }

    dropUnconnectedBalls(true);


    bool lineUsed;
    numLines=0;
    //counting the actual remaining lines
    for ( row = 0 ; row < WELL_HEIGHT; row++)
    {
        int rowLength = maxBalls(row);
        lineUsed=false;

        for ( col = 0; col < rowLength; col++ )
        {
            if (well[row][col]!=NULL)
            {
                lineUsed=true;
                break;//a ball exists in this row
            }
        }
        if (lineUsed) numLines++;
        else break;// found an empty row
    }


    int steps = 0;//drop the piston some...
    if (numLines<=WELL_HEIGHT/2)
    {
        int drop_ratio=(numLines*(WELL_WIDTH-1.)+numLines/2.)/numStaticBalls;
        steps=(drop_ratio-1+(WELL_HEIGHT-2-numLines)/(1+levelGen.generateNext()%3))%( (WELL_HEIGHT-numLines)/2);
    }

    lowerTopPlate(steps);

    initial_step=piston_step;
}

//------------------------------------------------
void InGame::readLevel(int level)
{
    DEBUG_REC();

    char fname[70];
    sprintf(fname,"levels/width%02d/level%04d.bin",WELL_WIDTH,level);


    char buffer[MAX_WELL_HEIGHT*MAX_WELL_WIDTH+1];

    for (int i=0;i<MAX_WELL_HEIGHT*MAX_WELL_WIDTH+1;i++)
        buffer[i]=0;

    if (loadRawLevel(fname,buffer))
    {
        numStaticBalls=0;//might need emptywell and resetTopPlate

        int bi=1;

        int row,col;

        for ( row = 0 ; row < WELL_HEIGHT; row++)
        {
            for ( col = 0; col < WELL_WIDTH; col++ )
            {
                if ( buffer[bi] > 0)
                {
                    createBall( GridPoint(row,col), buffer[bi] - 1) ;
                    numStaticBalls++;
                    colorStats[ buffer[bi] -1 ]++;
                }
                bi++;
            }
        }

        if (( buffer[0] >0) && ( buffer[0] <WELL_HEIGHT))
            lowerTopPlate(buffer[0]);
    }

    else
#ifdef RANDOM_LEVELS
      if (isLegitimateCopy)
        generateLevel(level);
#else
        if (level !=0)
        {
            readLevel(0);
            gameLevel=0;
        }
#endif
    initial_step = piston_step;
    numInitialBalls = numStaticBalls;

}

//--------------------------------------------------------------------

void InGame::saveLevel()
{
    DEBUG_REC();

    char fname[70];
    sprintf(fname,"levels/width%02d/level%04d.bin",WELL_WIDTH,gameLevel);

    char buffer[MAX_WELL_HEIGHT*MAX_WELL_WIDTH +1];

    makeBufferFromLevel(buffer);

    saveRawLevel(fname,buffer);

}
//---------------------------------------------------------

void InGame::makeBufferFromLevel( char* buffer)
{
    DEBUG_REC();

    buffer[0]=piston_step;

    int bi=1;

    for (int row=0;row < WELL_HEIGHT;row++)
    {
        int col;
        for ( col=0; col< WELL_WIDTH; col++)
        {
            buffer[bi]=0;
            if ( well[row][col] != NULL )
                buffer[bi]= well[row][col]->getColor()+1;
            bi++;
        }
    }

}
//-GAMESTATE I/O-------------------------------------------
void InGame::saveGameState()
{
    DEBUG_REC();

    SDL_RWops *out=NULL;


    char fname[500];
    sprintf(fname,"%s/%s%02d.sav",glbAppPath,playerStats->getPlayerName().c_str(),WELL_WIDTH);

    out =SDL_RWFromFile(fname,"w");

    if (!out)
    {
#ifdef CONSOLE_LOG
        sprintf(glbLogMsg,"ERROR CREATING %s\n",fname);
        mySDL_Log(glbLogMsg);
#endif
        return;
    }


    char levelBuffer[MAX_WELL_HEIGHT*MAX_WELL_WIDTH +1];

    makeBufferFromLevel(levelBuffer);

    SDL_RWwrite( out, levelBuffer, sizeof(char),WELL_HEIGHT*WELL_WIDTH +1);

    int tmp=0;

    std::list<BallInfo*>::const_iterator li;

    for( li= fallingBalls.begin(); li != fallingBalls.end();li++)
    {
        if (*li ==NULL ) continue;

        tmp++;
    }

    SDL_RWwrite(out,&tmp,sizeof(int),1);

    for( li= fallingBalls.begin(); li != fallingBalls.end();li++)
    {
        if (*li ==NULL ) continue;

        (*li)->saveState(out);
    }

    int statBuffer[SAVE_BUFFER_SIZE];

    statBuffer[0]=gameSettings->randomness;
    statBuffer[1]=gameSettings->pressure;
    statBuffer[2]=gameLevel;
    statBuffer[3]=score;

    statBuffer[4]=roundScore;
    statBuffer[5]=angle;

    statBuffer[6]=roundTimer.GetTicks();
    statBuffer[7]=shotTimer.GetTicks();

    statBuffer[8]=currentBall->getColor();
    statBuffer[9]=nextBall->getColor();

    statBuffer[10]=playerStats->getLevelShots();
    statBuffer[11]=initial_step;
    statBuffer[12]=powerBallGenerated;
    statBuffer[13]=gameSettings->useBombs;
    statBuffer[14]=gameSettings->useChromeBalls;
    statBuffer[15]=gameSettings->useLevelers;
    statBuffer[16]=gameSettings->usePainters;
    statBuffer[17]=gameSettings->useShifters;
    statBuffer[18]=timeStopActive;
    statBuffer[19]=timeStopUsed;
    statBuffer[20]=ballToSummon;
    statBuffer[21]=ballSummoned;
    statBuffer[22]=match4;
    statBuffer[23]=match5;
    statBuffer[24]=match6;
    statBuffer[25]=numInitialBalls;
    statBuffer[26]=gameSettings->matchNumber;
    statBuffer[27]=startSkin;


    SDL_RWwrite( out, statBuffer, sizeof(int), SAVE_BUFFER_SIZE);

    SDL_RWclose(out);

#ifdef CONSOLE_LOG
    sprintf(glbLogMsg,"saved %s\n",fname);
    mySDL_Log(glbLogMsg);
#endif
}
//--------------------------------------------------------
bool InGame::loadGameState()
{
    DEBUG_REC();

    SDL_RWops *in=NULL;


    char fname[500];
    sprintf(fname,"%s/%s%02d.sav",glbAppPath,playerStats->getPlayerName().c_str(),WELL_WIDTH);

    in =SDL_RWFromFile(fname,"r");

    if (!in)
    {
#ifdef CONSOLE_LOG
        sprintf(glbLogMsg,"ERROR LOADING %s\n",fname);
        mySDL_Log(glbLogMsg);
#endif
        return false;
    }


    char levelBuffer[MAX_WELL_HEIGHT*MAX_WELL_WIDTH+1];

    for (int i=0;i<WELL_HEIGHT*WELL_WIDTH+1;i++)
        levelBuffer[i]=0;

    levelBuffer[27] = gameSettings->skin;


    SDL_RWread( in, levelBuffer, sizeof(char), WELL_HEIGHT*WELL_WIDTH+1 );

    numStaticBalls=0;//might need emptywell and resetTopPlate
    int bi=1;


    for ( int row = 0 ; row < WELL_HEIGHT; row++)
    {
        for ( int col = 0; col < WELL_WIDTH; col++ )
        {
            if ( levelBuffer[bi] > 0)
            {
                createBall( GridPoint(row,col), levelBuffer[bi] - 1) ;
                numStaticBalls++;
                colorStats[ levelBuffer[bi] -1 ]++;
            }
            bi++;
        }
    }

    if (( levelBuffer[0] >0) && ( levelBuffer[0] <WELL_HEIGHT))
        lowerTopPlate(levelBuffer[0]);

    int numFallingBalls=0;

    SDL_RWread(in,&numFallingBalls,sizeof(int),1);//number of falling balls

    BallInfo* ball;

    for (int i=0;i<numFallingBalls;i++)
    {
        ball=new BallInfo(ball_radius,FPoint(0,0),COLOR_WHITE,BALLSTATE_FALLING);
        ball->loadState(in);
        fallingBalls.push_front(ball);
        ball=NULL;
    }

    int statBuffer[SAVE_BUFFER_SIZE];

    for (int i=0;i<SAVE_BUFFER_SIZE;i++)
        statBuffer[i]=0;

    SDL_RWread( in, statBuffer, sizeof(int), SAVE_BUFFER_SIZE);
    SDL_RWclose(in);

    if (gameSettings->overrideSettingsOnContinue)
    {

        gameSettings->randomness=statBuffer[0];
        gameSettings->pressure=statBuffer[1];

        gameSettings->useBombs =(statBuffer[13]?true:false);
        gameSettings->useChromeBalls =(statBuffer[14]?true:false);
        gameSettings->useLevelers = (statBuffer[15]?true:false);
        gameSettings->usePainters = (statBuffer[16]?true:false);
        gameSettings->useShifters = (statBuffer[17]?true:false);
        gameSettings->matchNumber = (statBuffer[26]>1)?statBuffer[26]:3;// <2 is unacceptable


    }
    else
        if (statBuffer[1]!=gameSettings->pressure)// would lead to abnormal piston relocation
        {
            gameLevel=statBuffer[2];  //load the level number and play from the beginning
            emptyWell();
            fallingBalls.clear();
            return false;
        }



    gameLevel=statBuffer[2];
    score=statBuffer[3];

    roundScore=statBuffer[4];
    angle=statBuffer[5];

    roundTimer.SetPausedTicks( statBuffer[6]-statBuffer[7] );
    shotTimer.SetPausedTicks(0);

    currentBall->setColor(statBuffer[8]);
    nextBall->setColor(statBuffer[9]);

    playerStats->setLevelShots(statBuffer[10]);

    initial_step=statBuffer[11];
    powerBallGenerated =(statBuffer[12]?true:false);



    timeStopActive            = (statBuffer[18]?true:false);
    timeStopUsed              = (statBuffer[19]?true:false);
    ballToSummon              = statBuffer[20];
    ballSummoned              = (statBuffer[21]?true:false);
    match4                    = statBuffer[22];
    match5                    = statBuffer[23];
    match6                    = statBuffer[24];
    numInitialBalls           = statBuffer[25];


    startSkin                   = statBuffer[27];

    setNextBallPos();

#ifdef CONSOLE_LOG
        sprintf(glbLogMsg,"read %s\n",fname);
        mySDL_Log(glbLogMsg);
#endif



    return true;
}
//--------------------------------------------------------------------

void InGame::createBall(const GridPoint &pt, int color,bool useEffects)
{
    DEBUG_REC();

    BallInfo* ball = new BallInfo( ball_radius, FPoint(getBallXCoord(pt.row,pt.col),getBallYCoord(pt.row)),
                                    color, BALLSTATE_STATIC, useEffects );

    well[pt.row][pt.col]=ball;

}

//-------------------------------------------------------------------------
void InGame::loadImages()
{
    DEBUG_REC();

    handles[COLOR_WHITE]=glbTexManager->insertTexture("ball_white.png",gameSettings->skin,true);
    handles[COLOR_BLACK]=glbTexManager->insertTexture("ball_black.png",gameSettings->skin,true);
    handles[COLOR_RED]=glbTexManager->insertTexture("ball_red.png",gameSettings->skin,true);
    handles[COLOR_GREEN]=glbTexManager->insertTexture("ball_green.png",gameSettings->skin,true);
    handles[COLOR_BLUE]=glbTexManager->insertTexture("ball_blue.png",gameSettings->skin,true);
    handles[COLOR_PURPLE]=glbTexManager->insertTexture("ball_purple.png",gameSettings->skin,true);
    handles[COLOR_YELLOW]=glbTexManager->insertTexture("ball_yellow.png",gameSettings->skin,true);
    handles[COLOR_ORANGE]=glbTexManager->insertTexture("ball_orange.png",gameSettings->skin,true);
    handles[COLOR_BOMB]=glbTexManager->insertTexture("balls/ball_bomb.png",SKIN_UNIVERSAL,true);
    handles[COLOR_SHIFTER]=glbTexManager->insertTexture("balls/ball_shifter.png",SKIN_UNIVERSAL,true);
    handles[COLOR_LEVELER]=glbTexManager->insertTexture("balls/ball_leveler.png",SKIN_UNIVERSAL,true);
    handles[COLOR_PAINTER]=glbTexManager->insertTexture("balls/ball_painter.png",SKIN_UNIVERSAL,true);
    handles[COLOR_CHROME]=glbTexManager->insertTexture("balls/ball_chrome.png",SKIN_UNIVERSAL,true);
    handles[COLOR_BLOCK]=glbTexManager->insertTexture("balls/ball_block.png",SKIN_UNIVERSAL,true);
    handles[COLOR_REFLECTOR]=glbTexManager->insertTexture("balls/ball_reflector.png",SKIN_UNIVERSAL,true);

    handles[TEX_BALL_INDEX]=handles[COLOR_WHITE];//not used

    handles[TEX_WELL_BACKGROUND]=glbTexManager->insertTexture("well_background.png",gameSettings->skin,true);

    handles[TEX_MACHINE]=glbTexManager->insertTexture("machine.png", SKIN_UNIVERSAL,true);
    handles[TEX_CANNON]=glbTexManager->insertTexture("cannon.png",SKIN_UNIVERSAL,true);
    handles[TEX_WELL_BOTTOM]=glbTexManager->insertTexture("well_bottom.png",SKIN_UNIVERSAL,true);
    handles[TEX_WELL_TOP]=glbTexManager->insertTexture("well_top.png",SKIN_UNIVERSAL,true);
    handles[TEX_WELL_LEFT]=glbTexManager->insertTexture("well_left.png",SKIN_UNIVERSAL,true);
    handles[TEX_WELL_RIGHT]=glbTexManager->insertTexture("well_right.png",SKIN_UNIVERSAL,true);
    handles[TEX_WELL_LCORNER]=glbTexManager->insertTexture("well_corner_left.png",SKIN_UNIVERSAL,true);
    handles[TEX_WELL_RCORNER]=glbTexManager->insertTexture("well_corner_right.png",SKIN_UNIVERSAL,true);
    handles[TEX_WELL_PISTONS]=glbTexManager->insertTexture("well_pistons.png",SKIN_UNIVERSAL,true);
    handles[TEX_WELL_TOP_PLATE]=glbTexManager->insertTexture("well_top_plate.png",SKIN_UNIVERSAL,true);
    handles[TEX_WELL_ENDLINE]=glbTexManager->insertTexture("well_endline.png",SKIN_UNIVERSAL,true);
    handles[TEX_WELL_PISTON_BACKGROUND]=glbTexManager->insertTexture("well_piston_background.png",SKIN_UNIVERSAL,true);
    handles[TEX_TIMESTOP]=glbTexManager->insertTexture("balls/timestop.png",SKIN_UNIVERSAL,true);


    //more to follow

    //(glbTexManager->getTexturePtr(handles[TEX_BACKGROUND]))->useColorMod(true);
}

//-----------------------------------
void InGame::initCursor()
{
    DEBUG_REC();

    SDL_ShowCursor(SDL_ENABLE);
}

//------------------------------------------
void InGame::startCountDownTimer()
{
    DEBUG_REC();

    countDownTimer.Start();
}
//------------------------------------------
void InGame::initTimers()
{
    if (playState==MODE_PLAY)
    {
        roundTimer.Start();
        shotTimer.Start();
    }
}
//------------------------------------------
void InGame::pauseTimers()
{
    if (playState==MODE_PLAY)
    {
        roundTimer.Pause();
        shotTimer.Pause();
        if (countDownTimer.GetTicks()<1000)
            countDownTimer.Pause();
    }
    else
    if (playState==MODE_PREGAME)
        countDownTimer.Pause();
}
//------------------------------------------
void InGame::resumeTimers()
{
    DEBUG_REC();

    if (playState==MODE_PLAY)
    {
        roundTimer.Resume();
        shotTimer.Resume();
        if (countDownTimer.GetTicks()<1000)
            countDownTimer.Resume();
    }
    else
    if (playState==MODE_PREGAME)
        countDownTimer.Resume();

}
//------------------------------------------
void InGame::stopTimers()
{
    if (playState==MODE_PLAY)
    {
        roundTimer.Stop();
        shotTimer.Stop();
        countDownTimer.Stop();
    }
}
//---------------------------------------------

InGame::~InGame()
{
    DEBUG_REC();

#ifdef CONSOLE_LOG
    sprintf(glbLogMsg,"InGame: Destructor: State: [%d] Prev: [%d]\n",playState,prevState);
    mySDL_Log(glbLogMsg);
#endif // CONSOLE_LOG
    if (
        ( (playState!=MODE_GAME_MENU) && (playState!=MODE_LEVEL_SELECT)&& (playState!=MODE_PREGAME)&& (playState!=MODE_ENDGAME))
        &&
        (
         (
          (playState==MODE_PAUSE)||(playState==MODE_QUIT)) &&
          ((prevState!=MODE_GAME_MENU) && (prevState!=MODE_LEVEL_SELECT)&& (prevState!=MODE_PREGAME)&& (prevState!=MODE_ENDGAME))
         )
        )
        saveGameState();
    else
    {

        if (roundTimer.GetTicks()>0)
        {
            int duration = roundTimer.GetTicks();
            playerStats->addPlayTime(duration);
        }

    }

    stopTimers();

    emptyWell();

    deleteFallingBalls();

    fallingBalls.clear();
    ballTuple.clear();

    delete nextBall;
    delete currentBall;

    delete texCountDown;
    delete texMessage;
    delete texLevel;
    //delete texScore;

    if (texPaused!=NULL)
        delete texPaused;

    if (texBallIndex!=NULL)
        delete texBallIndex;

    if (texWellHardware!=NULL)
        delete texWellHardware;
    if (texStaticBalls!=NULL)
        delete texStaticBalls;
    if (texShiningBalls!=NULL)
        delete texShiningBalls;
    if (texLauncher!=NULL)
        delete texLauncher;

    if (levelSelector!=NULL)
        delete levelSelector;

    if (gameMenu!=NULL)
        delete gameMenu;
    if (summonMenu!=NULL)
        delete summonMenu;

    if (endGame!=NULL)
        delete endGame;

    if (pauseScreen!=NULL)
        delete pauseScreen;

    for (int i=0;i<3;i++)
    {
            if (texRoundScore[i]!=NULL)
                    delete texRoundScore[i];
            if (texRoundTime[i]!=NULL)
                delete texRoundTime[i];
            if (texRoundShots[i]!=NULL)
                delete texRoundShots[i];
    }

    if (largeMessageFont!=NULL)
        TTF_CloseFont( largeMessageFont );
    if (mediumMessageFont!=NULL)
        TTF_CloseFont( mediumMessageFont );
    if (smallMessageFont!=NULL)
        TTF_CloseFont( smallMessageFont );

    if (flashEffect!=NULL)
        delete flashEffect;

    disableCursor();

    endSound();

    unloadImages();

    for (int i=0; i<NUM_TEXTURES;i++)
        delete recs[i];

}

//-----------------------------------

void InGame::unloadImages()
{
    DEBUG_REC();

   //for (int i=0; i<NUM_TEXTURES;i++)
     //       glbTexManager->removeTexture(handles[i]);
}

//--------------------------------------

void InGame::disableCursor()
{
    DEBUG_REC();

    SDL_ShowCursor(SDL_DISABLE);
}

//-----------------------------------

void InGame::endSound()
{
    DEBUG_REC();

 //   Mix_HaltChannel(-1);
 //   Mix_FreeChunk(sndClick);
}

//-----------------------------------------------

void InGame::HandleEvents()
{
    DEBUG_REC();

    bool bRotation=false;

    SDL_Event event;

    while (SDL_PollEvent(&event))
    {
        if ( (event.type==SDL_QUIT) || (event.type==SDL_APP_TERMINATING ))
            StateMachine::SetNextState(STATE_TITLE);

        else
            if ( (event.type==SDL_APP_WILLENTERBACKGROUND) ||
                 (event.type==SDL_APP_DIDENTERFOREGROUND ))
                 {
                     isHibernating=!isHibernating;

                     if (gameSettings->musicVolume > 0)
                     {
                        if (isHibernating) Mix_PauseMusic();
                            else Mix_ResumeMusic();
                     }

                     bool makeAggregates=false;

                    if ((playState==MODE_PAUSE)&&(prevState!=MODE_GAME_MENU) &&(prevState!=MODE_LEVEL_SELECT))
                    {
                        pauseGame(MODE_PAUSE,false);
                        makeAggregates=true;
                    }

                    pauseGame(MODE_PAUSE,makeAggregates);// android sends this when the graphics context is already lost, so no aggregate images are drawn. thus the double unpause-pause on wakeup.
                    #ifdef CONSOLE_LOG
                        sprintf(glbLogMsg,"InGame: HandleEvents:Focus change\n");
                        mySDL_Log(glbLogMsg);
                    #endif // CONSOLE_LOG
                 }

        else if( event.type == SDL_WINDOWEVENT )
            { //Window resize/orientation change
                if( event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED )
                {
                    #ifdef CONSOLE_LOG
                        sprintf(glbLogMsg,"InGame: HandleEvents:size change\n");
                          mySDL_Log(glbLogMsg);
                    #endif // CONSOLE_LOG

                    pauseTimers();//works only in play mode

                    if ( (event.window.data1!=screenRect->w) ||
                         (event.window.data2!=screenRect->h) )
                    {
                        setScreenRect(event.window.data1,event.window.data2);
                        glbEffect->resize();
                        SDL_RenderPresent( glbRenderer );

                        resizeComponents();
                    }


                    resumeTimers(); //works only in playmode
                }
            }
        else
        {
            bool handled =false;

            if (event.type==SDL_KEYDOWN)
            {
                switch (event.key.keysym.sym)
                {
                    case SDLK_AC_BACK:
                    case SDLK_ESCAPE: escapePressed();handled=true;break;
                    case SDLK_p     : pauseGame(MODE_PAUSE);handled=true;break;
                    case SDLK_n     : if (warnedForQuit)
                                        {
                                            pauseGame(MODE_QUIT);
                                            handled=true;
                                        }break;
                }
            }

            if (!handled)
                switch (playState)
                {
                    case MODE_PLAY:
                        bRotation=gameHandleEvents(event);break;
#ifndef PLATFORM_ANDROID
                    case MODE_EDITOR:
                        bRotation=editorHandleEvents(event);break;
#endif
                    case MODE_PREGAME:
                        bRotation=preGameHandleEvents(event);break;

                    case MODE_GAME_MENU:
                        gameMenuHandleEvents(event);break;
                    case MODE_LEVEL_SELECT:
                        levelSelectorHandleEvents(event);break;
                    case MODE_ENDGAME:
                        endGameHandleEvents(event);break;
                    case MODE_PAUSE:
                    case MODE_QUIT:
                        pauseScreenHandleEvents(event);break;
                    case MODE_SUMMON_MENU:
                        summonMenuHandleEvents(event);break;

                };
        }
    }

    if (
         (!bRotation) &&
         ( (playState==MODE_PLAY) || (playState==MODE_EDITOR) || (playState==MODE_PREGAME))
        )
        decelAngle();
}
//---------------------------------------------------------------------------
bool InGame::gameHandleEvents(SDL_Event& event)
{
    DEBUG_REC();

    PROFILER_START();
    bool bRotation=false;
    static bool bTouchedSummon=false;

    if (event.type==SDL_KEYDOWN)
    {
        switch (event.key.keysym.sym)
        {
//-----transition to editor stage
            case SDLK_e     : editorSeq = (editorSeq==0)?editorSeq+1:0; break;
            case SDLK_d     : editorSeq = (editorSeq==1)?editorSeq+1:0; break;
            case SDLK_i     : editorSeq = (editorSeq==2)?editorSeq+1:0; break;
            case SDLK_t     : editorSeq = (editorSeq==3)?editorSeq+1:0; break;
            case SDLK_o     : editorSeq = (editorSeq==4)?editorSeq+1:0; break;
            case SDLK_r     : if (editorSeq==5 )
                                { editorSeq=0; pauseTimers(); playState=MODE_EDITOR; initPalette(); }
                                break;
//in editor stage, choose ball color
            case SDLK_SPACE : launchBall();break;//testing

            case SDLK_LEFT  : decreaseAngle();accelAngle();bRotation=true;break;

            case SDLK_RIGHT : increaseAngle();accelAngle();bRotation=true;break;

            case SDLK_s     : summonMenuInit();break;

            default:editorSeq=0;break;
        };
    }
#ifndef PLATFORM_ANDROID
    else if (event.type==SDL_MOUSEMOTION)
    {

        FPoint dp(event.motion.x,event.motion.y);
        setAngle(dp);

        transformBallEffects(dp);

        glbEffect->moveEffect(FPoint( event.motion.x , event.motion.y ));

    }
    else if (event.type==SDL_MOUSEBUTTONDOWN )
    {
        FPoint dp(event.button.x,event.button.y);

        if (testBallCollision( nextBall->getPosition(),
                              dp,
                              ball_radius,0.)
            )
        {
           switchColors();
        }
        else if (testBallCollision(summonPoint,dp,ball_radius,0.))
                    summonMenuInit();
        else if (event.button.y <= launchPoint.y)
                    launchBall();
    }
#endif
#ifdef PLATFORM_ANDROID
    else if ( ( event.type==SDL_FINGERDOWN ) || ( event.type==SDL_FINGERMOTION) )
    {
            FPoint dp(event.tfinger.x*screenRect->w,event.tfinger.y*screenRect->h);

            if ( ( event.type==SDL_FINGERDOWN )&& testBallCollision(nextBall->getPosition(),dp,1.5*ball_radius,0.) )
            {
                switchColors();
                bTouchedSummon=false;
            }
            else if ( (
                       ( event.type==SDL_FINGERDOWN ) ||
                        (( event.type==SDL_FINGERMOTION)&&bTouchedSummon)
                       )
                     && testBallCollision(summonPoint,dp,1.5*ball_radius,0.) )
                        bTouchedSummon=true;
            else
            {
                setAngle(dp);
                bTouchedSummon=false;
            }

            glbEffect->moveEffect(dp);
            transformBallEffects(dp);

            fingerVel.x=screenRect->w*event.tfinger.dx;
            fingerVel.y=screenRect->h*event.tfinger.dy;

    }
    else if (event.type==SDL_FINGERUP)
    {
         FPoint dp(event.tfinger.x*screenRect->w,event.tfinger.y*screenRect->h);

        if ((bTouchedSummon) && (testBallCollision(summonPoint,dp,1.5*ball_radius,0.)) )
        {
            summonMenuInit();
            bTouchedSummon=false;
            touchHappened=false;
        }

        if ( touchHappened )
        {
             if (dp.y <= launchLimit )
             {
                 dp+=fingerVel;
                 //if finger exits screen while dragging, consider it unintentional,cancel shot
                 if (
                     ( (dp.x < ball_radius*0.725) ||
                        (dp.x > screenRect->w-ball_radius*0.725) ||
                        (dp.y < ball_radius*0.725) )
                    )
                    touchHappened=false;
                else
                        //if after auto shot user lifts the finger after dragging,
                    //and it happens too soon, cancel the shot, it's a wrong call
                 if ( (lastShotWasAuto)&&(shotTimer.GetTicks()<meanShotTime*400) )
                 {
                    touchHappened=false;
                    lastShotWasAuto=false;
                 }
                 else
                    if (justSwitched && testBallCollision(nextBall->getPosition(),dp,2.*ball_radius,0.))
                    {
                        touchHappened=false;
                        lastShotWasAuto=false;
                        justSwitched=false;
                    }
                    else
                        launchBall();
             }
        }
         fingerVel.x=fingerVel.y=0;
    }

#endif // PLATFORM_ANDROID
    PROFILER_STOP();

    return bRotation;
}

//---------------------------------------------------------------------------
void InGame::initPalette()
{
    int numPaletteObjects=NUM_BALLCOLORS-NUM_SPECIALBALLS+2;
    int ball_diam = guiUnit;
    int hor_spacing=2*rcWell.w/numPaletteObjects;

    FPoint offset(recs[TEX_WELL_ENDLINE]->x, recs[TEX_WELL_ENDLINE]->y+ball_diam/2);

    for (int i=0 ; i < numPaletteObjects ;i++)
    {
        palette[i].first=FPoint( offset.x + (0.5+i/2)*hor_spacing, offset.y+(i&1)*ball_diam);
        palette[i].second=i;
    }
    palette[numPaletteObjects-2].second=COLOR_BLOCK;
    palette[numPaletteObjects-1].second=COLOR_REFLECTOR;
    invTexLauncher=true;
}
//----------------------------------------------------------------------------
void InGame::editShiftBalls(int shift_type)//left or right
{
    if (numStaticBalls==0) return;
    bool usedrow;
    for (int r=0;r < WELL_HEIGHT-piston_step;r++ )
    {
        usedrow=false;
        int c;
        if (shift_type==NEIGHBOR_RIGHT)
        {
            if (well[r][maxBalls(r)-1]!=NULL)
            {
                dropBall(GridPoint(r,maxBalls(r)-1));
                usedrow=true;
            }

            for (c=maxBalls(r)-2;c>=0;c--)
            {
                if (well[r][c]!=NULL)
                {
                    well[r][c+1]=well[r][c];
                    well[r][c]=NULL;
                    usedrow=true;
                }
            }
        }
        else
        {
            if (well[r][0]!=NULL)
            {
                dropBall(GridPoint(r,0));
                usedrow=true;
            }

            for (c=1;c<maxBalls(r);c++)
            {
                if (well[r][c]!=NULL)
                {
                    well[r][c-1]=well[r][c];
                    well[r][c]=NULL;
                    usedrow=true;
                }
            }
        }
        if (!usedrow) break;
    }
    dropUnconnectedBalls();
    adjustStaticPositions();
    invTexStatic=true;
    invTexShining=true;

}
//---------------------------------------------------------------------------
bool InGame::editorHandleEvents(SDL_Event& event)
{
    DEBUG_REC();

    bool bRotation=false;

    if (event.type==SDL_KEYDOWN)
    {
        switch (event.key.keysym.sym)
        {
            case SDLK_UP    : raiseTopPlate(); invTexWellHardware=true;break;
            case SDLK_DOWN  : if ( lowerTopPlate() )
                              {
                                  GridPoint dr(WELL_HEIGHT-piston_step,0);
                                  for (int col=0;col<maxBalls( dr.row );col++)
                                  {
                                      dr.col=col;
                                      if (well[dr.row][dr.col]!=NULL)
                                        dropBall(dr);
                                  }

                                  invTexStatic=true;
                                  invTexShining=true;
                              }
                              invTexWellHardware=true;break;



            case SDLK_PAGEDOWN: {gameLevel++;initRound();break;}

            case SDLK_PAGEUP:   {   if (gameLevel>0) gameLevel--;
                                    initRound();
                                    break;   }
            case SDLK_s     : saveLevel();break;

            case SDLK_SPACE : launchBall();break;

            case SDLK_LEFT  : editShiftBalls(NEIGHBOR_LEFT);break;

            case SDLK_RIGHT : editShiftBalls(NEIGHBOR_RIGHT);break;

            default:break;
        };
    }
    else if ((event.type==SDL_MOUSEBUTTONDOWN ) || (event.type==SDL_MOUSEMOTION))
    {

     if (event.type==SDL_MOUSEMOTION)
     {
        currentBall->setPosition( FPoint( event.motion.x , event.motion.y ) );
        glbEffect->moveEffect( FPoint( event.motion.x , event.motion.y ) );

     }

     int mx,my;
     Uint8 mouseBtn=SDL_GetMouseState(&mx,&my);

     if (my <= recs[TEX_WELL_ENDLINE]->y)
        {
           GridPoint pt = findNearestGridPos(FPoint(mx,my));

           if (well[pt.row][pt.col]!=NULL)            {

                if (mouseBtn & SDL_BUTTON(3) )//( event.button.button==SDL_BUTTON_RIGHT)
                {
                    dropBall(pt);
                    dropUnconnectedBalls();

                    invTexStatic=true;//invalidate pre-rendered textures
                    invTexShining=true;
                }
                else if (
                         //(event.button.button==SDL_BUTTON_LEFT) &&
                          (mouseBtn & SDL_BUTTON(1)) && ( pt.row < (WELL_HEIGHT-piston_step) )
                         && ( well[pt.row][pt.col]->getColor()!=currentBall->getColor() )
                         )
                {
                    if (currentBall->getColor()!=COLOR_REFLECTOR)
                    {
                        colorStats[well[pt.row][pt.col]->getColor()]--;
                        well[pt.row][pt.col]->setColor( currentBall->getColor() );
                        colorStats[well[pt.row][pt.col]->getColor()]++;
                    }
                    else
                    {
                        dropBall(pt);
                        dropUnconnectedBalls();
                    }
                    invTexStatic=true;
                    invTexShining=true;//HEREHERE
                }
                else if (mouseBtn & SDL_BUTTON(2)  )//(event.button.button==SDL_BUTTON_MIDDLE)
                {
                    currentBall->setColor( well[pt.row][pt.col]->getColor() );
                }

            }
            else
                if (
                    ( mouseBtn & SDL_BUTTON(1))&&( pt.row < (WELL_HEIGHT-piston_step) )
                    && (currentBall->getColor()!=COLOR_REFLECTOR) )
                {
                    anchorBall(pt);

                    invTexStatic=true;
                    invTexShining=true;
                }

        }
        else
        {
            if ( mouseBtn & SDL_BUTTON(1) )
            {
                FPoint pt(mx,my);

                for (int i=0;i<NUM_BALLCOLORS-NUM_SPECIALBALLS+2;i++)
                {
                    if (testBallCollision(pt,palette[i].first,1,guiUnit/2))
                    {
                        currentBall->setColor(palette[i].second);
                        break;
                    }
                }
            }
        }
    }
    else if (event.type==SDL_MOUSEBUTTONUP )
    {
         dropUnconnectedBalls();
         invTexStatic=true;
         invTexShining=true;
    }
    else if (event.type==SDL_MOUSEWHEEL)
    {
        int numValidBalls=NUM_BALLCOLORS+NUM_OBSTACLES;//-NUM_SPECIALBALLS;
        if (event.wheel.y==1)
            currentBall->setColor( (currentBall->getColor()+1)%numValidBalls);
        else
        if (event.wheel.y==-1)
            currentBall->setColor( (currentBall->getColor()+numValidBalls-1)%numValidBalls );
    }

    return bRotation;
}

//--------------------------------------------------------------

bool InGame::preGameHandleEvents(SDL_Event& event)
{
    DEBUG_REC();

    bool bRotation=false;

    if (event.type==SDL_KEYDOWN)
    {
        switch (event.key.keysym.sym)
        {
            case SDLK_LEFT  : decreaseAngle();accelAngle();bRotation=true;break;

            case SDLK_RIGHT : increaseAngle();accelAngle();bRotation=true;break;
        };
    }
#ifndef PLATFORM_ANDROID
    else if (event.type==SDL_MOUSEMOTION)
    {
        FPoint dp(event.motion.x,event.motion.y);
        setAngle(dp);

        transformBallEffects(dp);

        glbEffect->moveEffect(FPoint( event.motion.x , event.motion.y ));
    }
#endif
#ifdef PLATFORM_ANDROID
    else if ( ( event.type==SDL_FINGERDOWN ) || ( event.type==SDL_FINGERMOTION) )
    {
            FPoint dp(event.tfinger.x*screenRect->w,event.tfinger.y*screenRect->h);

            setAngle(dp);

            glbEffect->moveEffect(dp);
            transformBallEffects(dp);
            touchHappened=false;
    }

#endif // PLATFORM_ANDROID

    return bRotation;
}
//--------------------------------------------------------------
void InGame::resizeComponents()
{
    DEBUG_REC();

    SDL_Rect rcView;

    if (playState==MODE_GAME_MENU)
        rcView={0,0, (int)(( guiBox->w - 0.5*guiUnit )/2.), (int)(( guiBox->h- 1.5*guiUnit )/2.)};
    else if (playState==MODE_LEVEL_SELECT)
        rcView={ 0, 0, (int)((guiBox->w - 0.45*guiUnit)/3.), (int)((guiBox->h - 0.45*guiUnit)/3.)};
    else
        rcView ={0,0,screenRect->w,screenRect->h};

    computeDimensions(&rcView);

    initFonts();
    //must change the fonts
/*
    if (playState==MODE_GAME_MENU)
        {
          //  gameMenu->resizeComponents();
        }
    else if (playState==MODE_LEVEL_SELECT)
        {
        //    levelSelector->resizeComponents();
        }
    else
    {
        updateLevelMessage();

        if (gameSettings->showRoundScore)
            updateRoundScoreMessage();
        else
            updateScoreMessage();
        if (gameSettings->showRoundShots)
            updateRoundShotsMessage();
        if (gameSettings->showRoundTime)
            updateRoundTimeMessage(true);

        updateStaticBallImage();
        updateShineEffectImage(;
        updateWellHardwareImage();
        updateCountDownMessage();
    }
    */
}
//------------input funcs---------------------------------------

void InGame::escapePressed()//DONE
{
    DEBUG_REC();

    switch (playState)
    {

        case MODE_PAUSE:
                pauseGame(MODE_PAUSE);
/*                warnedForQuit=false;
                playState=prevState;//escape undoes all pause screens
                delete pauseScreen;
                pauseScreen=NULL;
                if ( (texPaused!=NULL) &&(playState!=MODE_SUMMON_MENU))
                {
                    delete texPaused;
                    texPaused=NULL;
                }
                resumeTimers();*/
                break;

        case MODE_LEVEL_SELECT:
                delete levelSelector;
                levelSelector=NULL;
                gameMenuInit();
                break;

        case MODE_GAME_MENU:
                //game menu deleted by the destructor
                StateMachine::SetNextState(STATE_TITLE);
                break;

        case MODE_SUMMON_MENU:
                playState=MODE_PLAY;
                delete summonMenu;
                summonMenu=NULL;

                if (texPaused!=NULL)
                {
                    delete texPaused;
                    texPaused=NULL;
                }
                if (!timeStopActive)
                    resumeTimers();
                break;

        default:
                quitSequence();

    };
}

//---------------------------------------------------------

void InGame::quitSequence()//DONE
{
    DEBUG_REC();

    if (playState!=MODE_QUIT) warnedForQuit=false;

    if (warnedForQuit==false)
    {
        warnedForQuit=true;
        pauseGame(MODE_QUIT);
    }
    else StateMachine::SetNextState(STATE_TITLE);
}

//---------------------------------------------------------

void InGame::pauseGame(int mode,bool makeAggregates)//DONE
{
    DEBUG_REC();

    if (playState==mode)
    {
        playState=prevState;//undo state with the same key

        if (pauseScreen!=NULL)
        {
            delete pauseScreen;
            pauseScreen=NULL;

            if (playState!=MODE_SUMMON_MENU)
            {
                if (texPaused!=NULL)
                {
                    delete texPaused;
                    texPaused=NULL;
                }
            }
        }
        if ( (playState!=MODE_SUMMON_MENU) && (!timeStopActive) )
            resumeTimers();
       // Mix_ResumeMusic();
    }
    else if ( (playState==MODE_PLAY) ||(playState==MODE_PREGAME) ||
              (playState==MODE_ENDGAME ) ||(playState==MODE_PAUSE )
             ||(playState==MODE_EDITOR)|| (playState==MODE_SUMMON_MENU)
             )
    {
        pauseTimers();

        int rScore,time;

        if ( (playState==MODE_ENDGAME) || (prevState==MODE_ENDGAME) )
        {
            rScore=time=0;
        }
        else
        {
            time=roundTimer.GetTicks();
            rScore=roundScore;
        }

        if (playState==MODE_PAUSE)
        {
            delete pauseScreen;
            pauseScreen=NULL;
        }
        else
            prevState=playState;

        playState=mode;

        if (makeAggregates)
        {
            if (playState!=MODE_SUMMON_MENU)
                updatePausedImage();//it is already paused in this mode
        }

        pauseScreen = new PauseScreen(rcWell,gameLevel,playState==MODE_QUIT,prevState==MODE_PLAY,time,playerStats->getLevelShots(),rScore,makeAggregates);

    }
}
//===============CLASS SHOOTER=============================

void InGame::setAngle( FPoint dp)
{
    DEBUG_REC();

    PROFILER_START();
    float oldAngle=angle;
    //check if touch is inside valid playing space
    if (dp.y <= launchLimit)
    {
        dp-=launchPoint;
        double len=dp.GetLength();
        if (len>0)
        {
            angle = 90-(180/3.1415926)*acos( dp.x / len );
            if (angle > MAX_ANGLE)
                angle = MAX_ANGLE;
            else
                if (angle < -MAX_ANGLE)
                angle = - MAX_ANGLE;

            if (oldAngle!=angle)
            {
                invTexLauncher=true;
                if (gameSettings->useBallIndex)
                    computeBallIndex();
            }
#ifdef PLATFORM_ANDROID
            touchHappened=true;
#endif
        }
    }
#ifdef PLATFORM_ANDROID
    else touchHappened=false;
#endif
PROFILER_STOP();
}


//---------------------------------------------------------

void InGame::increaseAngle()//DONE
{
    DEBUG_REC();

    if (angle <MAX_ANGLE)
    {
        angle= std::min(angle+angleDiff,MAX_ANGLE);
        invTexLauncher=true;

        if (gameSettings->useBallIndex)
            computeBallIndex();
    }
}

//---------------------------------------------------------

void InGame::decreaseAngle()//DONE
{
    DEBUG_REC();

    if (angle > - MAX_ANGLE)
    {
        angle= std::max(-MAX_ANGLE,angle-angleDiff);
        invTexLauncher=true;
        if (gameSettings->useBallIndex)
            computeBallIndex();
    }
}

//---------------------------------------------------------

void InGame::accelAngle()//DONE
{
    DEBUG_REC();

    if ((angle > -MAX_ANGLE) && (angle< MAX_ANGLE))
    {
        if ( angleDiff < 10)
            angleDiff+=1;
    }
    else angleDiff=1.;
}
//---------------------------------------------------------
void InGame::decelAngle()//DONE
{
    DEBUG_REC();

    angleDiff/=1.1;
    if (angleDiff<1.)
        angleDiff=1.;
}
//----------------------------------------------------------
void InGame::computeBallIndex()
{
    DEBUG_REC();

    PROFILER_START();

    invTexBallIndex=true;

    bool stop =false;
    indexCollides=false;
    indexLength=BALL_INDEX_LENGTH;

    int ballColor=(currentBall==NULL)?COLOR_WHITE:currentBall->getColor();

    FPoint p  = launchPoint;
    float travelAngle =PI*(angle-90)/180.;

    FPoint dp = FPoint( myCosine( travelAngle ),mySine(travelAngle) );

    dp *= ball_radius*1.2;

    int i;

    emptyCollisionTestMatrix();

    for (i=0;i<BALL_INDEX_LENGTH;i++)
        ballIndex[i]=FPoint(0,0);

    int walls[2] = { rcWell.x+ rcWell.w -(int)ball_radius,
                     rcWell.x + (int)ball_radius};
    int wallX;

    i=0;

    while((i < BALL_INDEX_LENGTH) &&( !stop ) )//number of index points for ball trajectory
    {
        p += dp;

        wallX=0;

        if  (p.x >= walls[0])
            wallX = walls[0];
        else if (p.x<=walls[1])
            wallX = walls[1];

        if (wallX!=0)
        {
            if (ballColor==COLOR_CHROME)
                stop=true;
            else
                {
                    p.x = 2*wallX - p.x;
                    dp.x*=-1.;
                }
        }

        if (p.y < topSurfaceY + ball_radius )
        {
            stop=true;
        }

        if (ballColor!=COLOR_CHROME)
        {
            GridPoint pt= findNearestGridPos(p);

            if (testVertexNeighBors(p,dp,pt,1.,false))//unlimited time
            {
                stop=true;
            }
        }

        ballIndex[i] = p;
        i++;
    }

    if (stop)
    {

        indexLength=std::min(BALL_INDEX_LENGTH,i+1);
        p=ballIndex[i-1];

        resetCollisions();

        float new_dt = findWallCollision(1.,p,dp);

        if (ballColor!=COLOR_CHROME)
            findBallCollisions(new_dt,p,dp);
            //if there is a wall collision, no point in looking further

        if (numCollisions>0)
        {
            float mintime=collisions[0].dt;
            //int soonest=0;
            for (int k=1;k<numCollisions;k++)
            {
                if ( collisions[k].dt <= mintime )
                {
                    mintime=collisions[k].dt;
              //      soonest=k;
                }
            }
            p+=dp*mintime;
//===================================================================================================
/*
            GridPoint otherball=collisions[soonest].gp;

            if (well[otherball.row][otherball.col]->getColor()==COLOR_REFLECTOR)
            {


                FPoint icv(getBallXCoord(collisions[soonest].gp.row,collisions[soonest].gp.col) , getBallYCoord(collisions[soonest].gp.row));


                icv-=p;

                FPoint v= dp;

                v.ProjectOn(icv);

                dp-= v*2;//bouncing off ball

                stop=false;
            }
            else
            {
*/
                GridPoint pt = findNearestGridPos(p);

                if (ballColor==COLOR_CHROME)
                   ballIndex[indexLength-1] = p;
                else
                    ballIndex[indexLength-1] = FPoint( getBallXCoord(pt.row,pt.col), getBallYCoord(pt.row));

                indexCollides=true;


        }
    }

    PROFILER_STOP();
}
//------------------------------------------------
void InGame::launchBall()
{
    DEBUG_REC();

    if (currentBall->getState()==BALLSTATE_READY)
    {
        currentBall->setState(BALLSTATE_MOVING);
        currentBall->setVelocity(FPoint( velocityMultiplier*BALL_VELOCITY*myCosine(PI*(angle-90)/180.),
                                   velocityMultiplier*BALL_VELOCITY*mySine(PI*(angle-90)/180.) )
                            );

        playerStats->addShot();

        if (gameSettings->showRoundShots)
        {
            updateRoundShotsMessage();
            invTexWellHardware=true;
        }

        roundTimer.Pause();
#ifdef PLATFORM_ANDROID
        touchHappened = false;
        justSwitched =false;
        if (shotTimer.GetTicks()>=shotDuration)
            lastShotWasAuto=true;

#endif
        soundEffect(SFX_LAUNCH);
    }
}
//-------------------------------------------------
void InGame::slotNextBall()
{
    DEBUG_REC();

    initBalls(nextBall->getColor());

    if (playState==MODE_PLAY)
    {
        if (timeStopActive)
        {
            deactivateTimeStop();
            setSummonIndicator();
        }

        roundTimer.Resume();
        shotTimer.Start();
    }
    setNextBallPos();
}
//--------------------------------------------------
void InGame::setNextBallPos()
{
    DEBUG_REC();

    float diff=(WELL_WIDTH-1.)*ball_radius*(1.-shotTimer.GetTicks()*1./shotDuration);

    if (diff < 0) diff=0.f;

    nextBall->setPosition(FPoint(launchPoint.x + diff, launchPoint.y));
}
//===============EO CLASS SHOOTER=============================

//=========================GAMEPLAY_FUNCTIONS===========================
void InGame::Logic(float dt, float phase)
{
    DEBUG_REC();

    resetEffectFlags();

    countDownLogic(dt);

    switch (playState)
    {
        case MODE_PLAY:
        case MODE_PREGAME:
        case MODE_EDITOR:       gameLogic(dt,phase);break;

        case MODE_ENDGAME:      gameLogic(dt,phase);
                                endGameLogic(dt);break;

        case MODE_GAME_MENU:    gameMenuLogic(dt);break;

        case MODE_LEVEL_SELECT: levelSelectorLogic(dt);break;
        case MODE_PAUSE:
        case MODE_QUIT:         pauseScreenLogic(dt);break;

        case MODE_SUMMON_MENU: summonMenuLogic(dt);break;
    }

}
//----------------------------------------
void InGame::countDownLogic(float dt)
{
    DEBUG_REC();

    if ( (playState==MODE_PLAY) || (playState==MODE_EDITOR) )
    {
        if (countDownTimer.GetTicks()>1000)//for theGo! message
            countDownTimer.Pause();
    }
    else
        if (playState==MODE_PREGAME)
        {
            int ticks=countDownTimer.GetTicks();
            if ( ticks < 3000)
              updateCountDownMessage();
            else
            {
                countDownTimer.Stop();
                countDownTimer.Start();
                setGameMessage("Go!");
                playState=MODE_PLAY;
                if (!timeStopActive)
                    resumeTimers();
            }
        }

}
//----------------------------------------
void InGame::gameLogic(float dt, float phase)
{
    DEBUG_REC();

    PROFILER_START();

    #ifdef CONSOLE_LOG
        if (currentBall->getState()==BALLSTATE_MOVING)
        {
            sprintf(glbLogMsg,"FLYING: dt=%.5f\n",dt);
            mySDL_Log(glbLogMsg);
        }
    #endif // CONSOLE_LOG

    isEffectActive =flashEffect->update(dt,phase);//initializing for this frame;

    if (summonEffect!=NULL)
        summonEffect->update(dt);

    int oldScore =roundScore;
    if (currentBall->getState()==BALLSTATE_READY)
    {
        if (shotTimer.GetTicks() >shotDuration)
            launchBall();
        //piston action controlled only after ball landing

    }
    setNextBallPos();

    if (currentBall->getState()==BALLSTATE_MOVING)//in flight
    {
        GridPoint gp;

        do
        {
            resetCollisions();

            float new_dt = findWallCollision(dt,
                                             currentBall->getPosition(),
                                             currentBall->getVelocity());
            findBallCollisions(new_dt,
                              currentBall->getPosition(),
                              currentBall->getVelocity());//if there is a wall collision, no point in looking further

            if (numCollisions>0)
            {
                float mintime=collisions[0].dt;
                int soonest=0;
                for (int i=1;i<numCollisions;i++)
                {
                    if ( collisions[i].dt <= mintime )
                    {
                        mintime=collisions[i].dt;
                        soonest=i;
                    }
                }
                moveFallingBalls(mintime);//some balls were in the falling
                                          //list before new ones were added,
                                          // these must move for the total time
                currentBall->update(mintime);

                updateBallEffects(mintime,phase);

                gp = doCollision(soonest);//side wall=bounce, top or ball finds anchor point and goes static
                if (currentBall->getColor()==COLOR_CHROME)
                {
                    bool update=true;

                    if (currentBall->getState()==BALLSTATE_MOVING)//it might connect to a wall with no neighbors
                        update=dropUnconnectedBalls();//drop any resulting unconnected balls

                    if (update)
                    {
                        flashEffect->shine(0.1,&rcWell);

                        invTexStatic=true;
                        invTexShining=true;
                    }
                }
                else
                    if ( (currentBall->getColor()==COLOR_PAINTER) &&
                         (currentBall->getState()==BALLSTATE_STATIC))
                        activatePainter(gp,soonest);
                dt-=mintime;

            }
            else
            {
                moveFallingBalls(dt);
                currentBall->update(dt);

                updateBallEffects(dt,phase);
                dt=0;
            }
        }
        while ( (dt>0) && ( currentBall->getState()==BALLSTATE_MOVING) );


         if (currentBall->getState()==BALLSTATE_STATIC)//anchored in well , a collision happened
        {
            slotNextBall();
            bool shine=false;
            bool obliterator=false;
            float duration=0.15;

            switch (well[gp.row][gp.col]->getColor())
            {
                case COLOR_BOMB :
                                    dropBombAndNeighbors( gp );
                                    shine=true;
                                    break;
                case COLOR_CHROME:
                                    duration=0.3;
                case COLOR_PAINTER:
                                    well[gp.row][gp.col]->explodeWithSparks();
                                    dropBall(gp,SCORE_BOMB);
                                    shine=true;
                                    break;
                case COLOR_SHIFTER:
                                    activateColorShifter(gp);
                                    shine=true;
                                    break;
                case COLOR_LEVELER:
                                    activateLeveler(gp);
                                    shine=true;
                                    obliterator=true;
                                    duration=0.3;
                                    break;

                default:
                    if (playState==MODE_PLAY)
                         dropSameColored( gp ); //find tuple of size >= (match number) and drop it
            }

            if (shine)
            {
                if (obliterator)
                {
                    FPoint p( getBallXCoord(gp.row,gp.col), getBallYCoord(gp.row) );
                    flashEffect->shine(duration,&rcWell,p,ball_radius);
                }
                else
                    flashEffect->shine(duration,&rcWell);
            }

#ifdef CONSOLE_LOG
            sprintf(glbLogMsg,"Dropping unconnected balls\n");
            mySDL_Log(glbLogMsg);
#endif
            dropUnconnectedBalls(false,(obliterator?gp:GridPoint(-1,-1)));//drop any resulting unconnected balls

            invTexStatic=true;
            invTexShining=true;

            checkTopPlateAction();

            if (gameSettings->useBallIndex)
                computeBallIndex();

            if (numStaticBalls==0)
                endLevel(true);


            if ( (gp.row==WELL_HEIGHT-piston_step) &&(well[gp.row][gp.col]!=NULL))//did not drop
            {
                endLevel(false);
            }

        }
    }
    if (dt>0)
    {

        moveFallingBalls(dt);//falling balls will drop for
                             //the time remaining after the collision
        updateBallEffects(dt,phase);
    }

    removeVanishedBalls();



    if (roundScore!=oldScore)
    {
        if (gameSettings->showRoundScore)
                updateRoundScoreMessage();
       // else
       //     updateScoreMessage();
        invTexWellHardware=true;
    }


    if (invTexStatic) updateStaticBallImage();

    if (invTexShining) updateShineEffectImage();

    if ((gameSettings->useBallIndex) && (invTexBallIndex)) updateBallIndexImage();

    if (invTexWellHardware) updateWellHardwareImage();

    if (invTexLauncher) updateLauncherImage();

    if (gameSettings->showRoundTime)  updateRoundTimeMessage();

    PROFILER_STOP();
}
//------------------------------------------------------------
void InGame::activateColorShifter( const GridPoint& gp)
{
    int newColor=findMaxTupleColor(gp);

    if (newColor==COLOR_SHIFTER)//was alone
    {
        soundEffect(SFX_LANDFAIL);
         well[gp.row][gp.col]->explodeWithSparks();
         dropBall(gp,SCORE_BOMB);
    }
    else
    {
        soundEffect(SFX_COLORSHIFTER);

        if (playState==MODE_PLAY)
            dropSameColored( gp,newColor); //find tuple of size >= (match number) and drop it

        if (well[gp.row][gp.col]!=NULL)
        {
            colorStats[COLOR_SHIFTER]--;
            well[gp.row][gp.col]->setColor(newColor);//did not drop
            colorStats[newColor]++;

            well[gp.row][gp.col]->explodeWithSparks();
        }
    }
}
//------------------------------------------------------------

void InGame::activateLeveler( const GridPoint& gp)
{
    int scr=0;

    soundEffect(SFX_LEVELER);

    for (int col=0; col<maxBalls(gp.row); col++)
        if (well[gp.row][col]!=NULL)
        {
            if (gameSettings->multiBallExplosions)
                well[gp.row][col]->explodeWithSparks();
            else
                well[gp.row][col]->explode();

            if (col!=gp.col)
            {
                dropBall(GridPoint(gp.row,col),SCORE_EXPLODED,gp);
                scr+=SCORE_EXPLODED;
            }
            else
            {
                dropBall(GridPoint(gp.row,col),SCORE_BOMB);
            }

        }
    roundScore+=scr;
}
//------------------------------------------------------------
void InGame::activatePainter( const GridPoint& gp,int collisionIndex)
{
    int newColor=COLOR_WHITE;
    bool found=false;

    if (collisions[collisionIndex].type==TYPE_BALL)
    {
        GridPoint pt=collisions[collisionIndex].gp;

        newColor = well[pt.row][pt.col]->getColor();
        found=true;
#ifdef CONSOLE_LOG
        sprintf(glbLogMsg,"BallCollision with color: %d",newColor);
        mySDL_Log(glbLogMsg);
#endif
    }
    else//searching for neighbors
    {

        for (int col=gp.col-1; (col<=gp.col+1)&&(!found);col+=2 )
            if (   ( (col>=0) && (col<maxBalls(0)) ) &&
                (well[0][col]!=NULL) )
            {
                found=true;
                newColor=well[0][col]->getColor();
            }
    }

    if (!found)
    {
        soundEffect(SFX_LANDFAIL);
        return;
    }
    soundEffect(SFX_PAINTER);

    std::vector<GridPoint> vp(0);
    initMap();
    BFS(gp,2, (WELL_WIDTH*WELL_HEIGHT)/12, vp);

    for (int i=0;i<vp.size();i++)
    {
        if (well[vp[i].row][vp[i].col]->getColor() != newColor)
        {
            colorStats[ well[vp[i].row][vp[i].col]->getColor() ]--;
            well[vp[i].row][vp[i].col]->setColor(newColor);
            colorStats[ well[vp[i].row][vp[i].col]->getColor() ]++;

            if (gameSettings->multiBallExplosions)
                well[vp[i].row][vp[i].col]->explodeWithSparks();
            else
                well[vp[i].row][vp[i].col]->explode();

#ifdef CONSOLE_LOG
        sprintf(glbLogMsg,"%d: Painting: %d %d\n",i+1,vp[i].row,vp[i].col);
        mySDL_Log(glbLogMsg);

#endif // CONSOLE_LOG

        }
    }

    if (vp.size()>0)
    {
        invTexStatic=true;
        invTexShining=true;
    }

    vp.clear();
}
//------------------------------------------------------------

void InGame::BFS(const GridPoint& gp,int maxDepth, int maxballs, std::vector<GridPoint> &pv)
{
    conmap[gp.row][gp.col]=1;

    //sprintf(glbLogMsg,"BFS: Start at %d %d\n",gp.row,gp.col);
    //mySDL_Log(glbLogMsg);

    int depth=0;
    bool stop=false;
    int balls=0;

    GridPoint cp=gp;
    int index=0;

    while ( ((depth<maxDepth) || (balls<maxballs)) &&(!stop))
    {
        int ncol;

        for (int i=-1;i<2;i++)//relative row
        {
            if ( (cp.row+i >=0) && ( (cp.row+i) < getWellMatrixHeight() ) )

            for ( int n = NEIGHBOR_LEFT; n <= NEIGHBOR_RIGHT; n++ )//neighbor on row
            {
                ncol=xIndex( cp,i,n);

                if ( (ncol >=0 ) &&( ncol < maxBalls(cp.row+i) ) )
                {
                    if (conmap[cp.row+i][ncol]==0)//non-visited
                    {
                        conmap[cp.row+i][ncol]=depth+1;
                        pv.push_back(GridPoint(cp.row+i,ncol));
                        balls++;

                       // sprintf(glbLogMsg,"BFS: Added ball#%d [%d , %d] at depth %d\n",balls, cp.row+i,ncol,depth+1);
                       // mySDL_Log(glbLogMsg);

                    }
                }
            }
        }
        if (index==pv.size())
            stop=true;
        else
        {
            cp=pv[index++];
            depth=conmap[cp.row][cp.col];
        }
    }


}

//------------------------------------------------------------
void InGame::dropBombAndNeighbors( const GridPoint &gp )
{
    DEBUG_REC();

    int ncol;
    int numNeighbors=0;

    for (int i=-1;i<2;i++)//relative row
    {
        if ( (gp.row+i >=0) && ( (gp.row+i) < getWellMatrixHeight() ) )

        for ( int n = NEIGHBOR_LEFT; n <= NEIGHBOR_RIGHT; n++ )//neighbor on row
        {
            ncol = xIndex( gp ,i,n);

            if ( (ncol >=0 ) && ( ncol < maxBalls(gp.row+i) ) )
            {
                if (well[gp.row+i][ncol]!=NULL)
                {

#ifdef CONSOLE_LOG
                    sprintf(glbLogMsg,"DROPNeighbors: Dropping %d,%d\n",gp.row+i,ncol);
                    mySDL_Log(glbLogMsg);
#endif
                    well[gp.row+i][ncol]->explodeWithSparks();

                    dropBall(GridPoint(gp.row+i,ncol),SCORE_EXPLODED,gp);
                    numNeighbors++;
                }
            }
        }
    }


    well[gp.row][gp.col]->explodeWithSparks();
    dropBall( gp,SCORE_BOMB );//plus the bomb itself
    soundEffect(SFX_BOMB);

    roundScore+=numNeighbors*SCORE_EXPLODED;
}
//-----------------------------------------------
int InGame::findMaxTupleColor(const GridPoint& p)
{
    int maxLength=1;
    int maxColor=COLOR_SHIFTER;

    for (int clr=0; clr < NUM_BALLCOLORS-NUM_SPECIALBALLS; clr++)
    {
        initMap();
        ballTuple.clear();
        findColorTuple(p , clr);
        int tupleSize=ballTuple.size();
        if (tupleSize>maxLength)
        {
            maxLength=tupleSize;
            maxColor=clr;
        }
    }
   ballTuple.clear();
   return maxColor;
}
//-----------------------------------------------
void InGame::dropSameColored(const GridPoint &gp,int color)
{
    DEBUG_REC();

    initMap();

    if (color==-1)
        color=well[gp.row][gp.col]->getColor();

    findColorTuple(gp,color);
    int tupleSize=ballTuple.size();
    int sc=0;

    if ( tupleSize >=gameSettings->matchNumber)
    {
        if (tupleSize==4) match4++;
        else if (tupleSize==5) match5++;
        else if (tupleSize>=6) match6++;

        std::list<GridPoint>::iterator i;
        for (i=ballTuple.begin();i!=ballTuple.end();++i)
        {
            well[(*i).row][(*i).col]->explode();
            if ( well[(*i).row][(*i).col]->getColor()==COLOR_SHIFTER )
            {
                well[(*i).row][(*i).col]->explodeWithSparks();
                dropBall(*i,SCORE_BOMB);
                sc += SCORE_BOMB;
            }
            else
            {
                dropBall(*i,SCORE_MATCHED);
                sc += SCORE_MATCHED;
            }

        }

        roundScore += sc;

    }
    ballTuple.clear();
}
//-----------------------------------------------------------------
void InGame::findColorTuple(const GridPoint &gp,int color)
{
    DEBUG_REC();

    PROFILER_START();
    ballTuple.push_back(gp);
    int ncol;

    conmap[gp.row][gp.col]=1;//visited

    for (int i=-1;i<2;i++)//relative row
    {
        if ( (gp.row+i >=0) && ( (gp.row+i) < getWellMatrixHeight() ) )

        for ( int n = NEIGHBOR_LEFT; n <= NEIGHBOR_RIGHT; n++ )//neighbor on row
        {
            ncol=xIndex(gp,i,n);

            if ( (ncol >=0 ) &&( ncol < maxBalls(gp.row+i) ) )
            {
                if ( ( conmap[ gp.row+i ][ ncol ] == 0) &&
                     ( well[ gp.row+i ][ ncol ]->getColor() == color)
                    )//non-visited
                    findColorTuple( GridPoint(gp.row+i,ncol),color);
            }
        }
    }
    PROFILER_STOP();
}

//------------------------------------------------------------
void InGame::moveFallingBalls(float dt)
{
    DEBUG_REC();
    if (fallingBalls.size()==0) return;
    std::list<BallInfo*>::iterator i;
    for( i= fallingBalls.begin(); i != fallingBalls.end();i++)
    {
        if (*i ==NULL ) continue;
        (*i)->update(dt);

        FPoint p=(*i)->getPosition();
        FPoint v=(*i)->getVelocity();
        bool change=false;

        if (p.y < topSurfaceY+ball_radius) { p.y=2*(topSurfaceY+ball_radius)-p.y; v.y*=-1.f;change=true;}
        if (p.x < rcWell.x+ball_radius) { p.x=2*(rcWell.x+ball_radius)-p.x; v.x*=-1.f;change=true;}
        else
            if (p.x > rcWell.x+rcWell.w-ball_radius) { p.x=2*(rcWell.x+rcWell.w-ball_radius)-p.x; v.x*=-1.f;change=true;}

        if (change)
        {
            (*i)->setPosition(p);
            (*i)->setVelocity(v);
            soundEffect(SFX_COLLISION);
        }

        (*i)->fade(-dt);
    }
}
//---------------------------------------------------------
void InGame::removeVanishedBalls()
{
    DEBUG_REC();

    PROFILER_START();
    bool foundBalls=false;
    std::list<BallInfo*>::iterator i;
    for( i= fallingBalls.begin(); i != fallingBalls.end();i++)
    {
        if (*i == NULL ) continue;

        if (isBallInvisible(*i))
        {
            delete *i;
            (*i)=NULL;
            soundEffect(SFX_BUBBLE);
            //fallingBalls.erase(i);//HERE ITERATOR INCOMPETENCE
#ifdef CONSOLE_LOG
        sprintf(glbLogMsg,"Falling Ball Erased\n");
        mySDL_Log(glbLogMsg);
#endif // CONSOLE_LOG

        }
        else
            foundBalls=true;
    }

    if (!foundBalls)
        if (!fallingBalls.empty())
        {
            fallingBalls.clear();
#ifdef CONSOLE_LOG
            sprintf(glbLogMsg,"Falling Ball List Cleared\n");
            mySDL_Log(glbLogMsg);
#endif // CONSOLE_LOG
        }
    PROFILER_STOP();
}
//---------------------------------------------------------
void InGame::updateBallEffects(float dt, float phase)
{
    DEBUG_REC();

    PROFILER_START();

    BallInfo* ball=NULL;

    for (int row=0; row < getWellMatrixHeight(); row++)
        for ( int col=0; col<maxBalls(row); col++)
        {
            ball = well[row][col];
            if ( ball != NULL)
                isEffectActive |= ball->updateEffects(dt,phase);
        }
    ball=NULL;
    if (!fallingBalls.empty())
    {
        std::list<BallInfo*>::iterator i;
        for( i= fallingBalls.begin(); i != fallingBalls.end();i++)
        {

            if (*i ==NULL ) continue;
            else isEffectActive |= (*i)->updateEffects(dt,phase);
        }
    }

    isEffectActive |= nextBall->updateEffects(dt,phase);

    isEffectActive |= currentBall->updateEffects(dt,phase);
    PROFILER_STOP();
}
//-----------------------------------------------

void InGame::transformBallEffects(const FPoint &pt)
{
    DEBUG_REC();

    PROFILER_START();
    if (gameSettings->useShineEffect)
        ShineEffect::setTouchLocation(pt);

    BallInfo* ball=NULL;

    for (int row=0; row < getWellMatrixHeight(); row++)
        for ( int col=0; col<maxBalls(row); col++)
        {
            ball = well[row][col];
            if ( ball != NULL)
                ball->transformEffects(pt);
        }
    ball=NULL;

    invTexShining=true;
                      /*//I think there are no transformable effects on the falling balls
    std::list<BallInfo*>::iterator i;
    for( i= fallingBalls.begin(); i != fallingBalls.end();i++)
    {
        ball=*i;
        if (ball ==NULL ) continue;
        else ball->transformEffects(pt);
    }
*/
    nextBall->transformEffects(pt);

    currentBall->transformEffects(pt);
    PROFILER_STOP();
}

//===============CLASS FREEBALL=============================
//---------------------------------------------------------
bool InGame::isBallInvisible(BallInfo* b)
{
    DEBUG_REC();

    FPoint pos = b->getPosition();

    if (  (pos.y >= rcWell.y + rcWell.h + ball_radius) ||
          (pos.x <= rcWell.x - ball_radius) ||
          (pos.x >= rcWell.x + rcWell.w + ball_radius) ||
          (b->getAlpha() <10)
        )
            return true;

    return false;
}

//---------------------------------------------------------
GridPoint InGame::doCollision(int index)// performs the collision found earlier
{
    DEBUG_REC();

    lastVelocity=currentBall->getVelocity();

    GridPoint gp=findNearestGridPos(currentBall->getPosition());

    if (currentBall->getColor()!=COLOR_CHROME)
    {
        if (collisions[index].type==TYPE_WELL_SIDE)
        {
            FPoint v=currentBall->getVelocity();
            v.x*=-1;
            currentBall->setVelocity(v);
            lastVelocity=v;
            soundEffect(SFX_COLLISION);
        }
        else
        {

            GridPoint otherball=collisions[index].gp;

            if ((collisions[index].type==TYPE_BALL)&&(well[otherball.row][otherball.col]->getColor()==COLOR_REFLECTOR))
            {


                FPoint dp(getBallXCoord(collisions[index].gp.row,collisions[index].gp.col) , getBallYCoord(collisions[index].gp.row));

                if (collisions[index].dt<0)
                {
                    sprintf(glbLogMsg,"Reflector Negative collision at time %.5f\n",collisions[index].dt);
                    mySDL_Log(glbLogMsg);

                    return gp;
                }

                dp-=currentBall->getPosition();

                FPoint v= currentBall->getVelocity();

                v.ProjectOn(dp);

                currentBall->setVelocity( (currentBall->getVelocity()-v*2));//bouncing off ball
                soundEffect(SFX_COLLISION);
            }
            else
               anchorBall(gp);//all balls except chrome and reflector

        }
    }
    else //chromeball has inverse behavior
    {

        if ( (collisions[index].type==TYPE_WELL_SIDE) || (collisions[index].type==TYPE_WELL_TOP) )
        {
            soundEffect(SFX_CHROME);
            anchorBall(gp);
        }

        else//ball collision
            {
                if (gameSettings->multiBallExplosions)
                    well[collisions[index].gp.row][collisions[index].gp.col]->explodeWithSparks();
                else
                    well[collisions[index].gp.row][collisions[index].gp.col]->explode();

                dropBall(collisions[index].gp,SCORE_EXPLODED,gp);
            }

    }
    return gp;
}
//---------------------------------------------------------
void InGame::anchorBall(const GridPoint& pt)
{
    DEBUG_REC();

    currentBall->setState(BALLSTATE_STATIC);//to signify anchoring
    currentBall->setVelocity(FPoint(0.,0.));
    currentBall->setPosition(FPoint(getBallXCoord(pt.row,pt.col),getBallYCoord(pt.row)));

    well[pt.row][pt.col]=currentBall;

    int color=currentBall->getColor();

    if (color< NUM_BALLCOLORS-NUM_SPECIALBALLS)
        colorStats[color]++;

    currentBall=new BallInfo(ball_radius,
                                launchPoint,
                                well[pt.row][pt.col]->getColor(),
                                BALLSTATE_STATIC
                                );

    numStaticBalls++;

    soundEffect(SFX_LAND);

#ifdef CONSOLE_LOG
    sprintf(glbLogMsg,"Anchored current ball to %d,%d\n",pt.row,pt.col);
    mySDL_Log(glbLogMsg);
#endif // CONSOLE_LOG

}
//---------------------------------------------------------
void InGame::resetCollisions()
{
    DEBUG_REC();

    numCollisions=0;
    for (int i=0;i<MAX_COLLISIONS;i++)
    {
        collisions[i].dt=100.;
    }
}
//---------------------------------------------------------
float InGame::findWallCollision(float dt,const FPoint& ballPos, const FPoint& ballVel)
{
    DEBUG_REC();

    FPoint currentPos=ballPos;
    FPoint currentVel=ballVel;

   FPoint posAtDt=currentPos + currentVel*dt;

   float x0,y0,collisionTime,mintime;

    mintime = dt;
    x0 = rcWell.x + ball_radius;
    if (posAtDt.x <= x0)
    {
        collisionTime=(x0-currentPos.x)/(1.f*currentVel.x);//time collision happens

        if (collisionTime<=dt)
        {
            collisions[ numCollisions ].type = TYPE_WELL_SIDE;
            collisions[ numCollisions ].dt   = collisionTime;
            collisions[ numCollisions ].gp.row=-1;
            collisions[ numCollisions ].gp.col=NEIGHBOR_LEFT; //signifying the wall
            numCollisions++;

            mintime=collisionTime;
#ifdef CONSOLE_LOG
        sprintf(glbLogMsg," Collision added: left wall at ( %.6f )\n",collisionTime);
        mySDL_Log(glbLogMsg);
#endif // CONSOLE_LOG
        }
        else
        {
#ifdef CONSOLE_LOG
        sprintf(glbLogMsg," Collision IGNORED: left wall at ( %.6f )\n",collisionTime);
        mySDL_Log(glbLogMsg);
#endif // CONSOLE_LOG

        }
    }
    else
    {
        x0=rcWell.x +rcWell.w - ball_radius;
        if (posAtDt.x >= x0)
        {

            collisionTime=(x0-currentPos.x)/(1.f*currentVel.x);
            if (collisionTime<=dt)
            {
                collisions[ numCollisions ].type = TYPE_WELL_SIDE;
                collisions[ numCollisions ].dt   = collisionTime;
                collisions[ numCollisions ].gp.row=-1;
                collisions[ numCollisions ].gp.col=NEIGHBOR_RIGHT; //signifying the wall
                numCollisions++;

                mintime=collisionTime;
#ifdef CONSOLE_LOG
                sprintf(glbLogMsg," Collision added: right wall at ( %.6f )\n",collisionTime);
                mySDL_Log(glbLogMsg);
#endif // CONSOLE_LOG
            }
            else
            {
#ifdef CONSOLE_LOG
                sprintf(glbLogMsg," Collision IGNORED: right wall at ( %.6f )\n",collisionTime);
                mySDL_Log(glbLogMsg);
#endif // CONSOLE_LOG
            }
        }
    }

    y0= topSurfaceY + ball_radius;
    if (posAtDt.y <= y0)
    {

        collisionTime=(y0-currentPos.y)/(1.f*currentVel.y);

        if (collisionTime<=dt)
        {
            collisions[ numCollisions ].type = TYPE_WELL_TOP;
            collisions[ numCollisions ].dt   = collisionTime;
            collisions[ numCollisions ].gp.row=-1;
            collisions[ numCollisions ].gp.col=-1; //signifying the top

            if (collisionTime<mintime)
                mintime=collisionTime;

            numCollisions++;
#ifdef CONSOLE_LOG
            sprintf(glbLogMsg," Collision added: top wall at ( %.6f )\n",collisionTime);
            mySDL_Log(glbLogMsg);
#endif // CONSOLE_LOG
        }
        else
        {
#ifdef CONSOLE_LOG
            sprintf(glbLogMsg," Collision IGNORED: top wall at ( %.6f )\n",collisionTime);
            mySDL_Log(glbLogMsg);
#endif // CONSOLE_LOG
        }
    }
return mintime;
}

//--------------------------------------------------------------
void InGame::findBallCollisions(float timeLimit,const FPoint& ballPos, const FPoint& ballVel)
{
    DEBUG_REC();

    //collisions are processed in a ordered manner, so the first (bunch) found is the soonest
    float t=0.;
    bool found=false;

    FPoint pos=ballPos;//currentBall->getPosition();
    float velLength= ballVel.GetLength();//currentBall->getVelocity().GetLength();

    GridPoint gp;

    emptyCollisionTestMatrix();

    float deltaT=timeLimit / (ceil(SQRT3*ball_radius/velLength));//did not exist

    while ((!found) && (t<=timeLimit))//HERE was <
    {
        gp = findNearestGridPos( pos );
        found=testVertexNeighBors( ballPos, ballVel,gp,timeLimit);

        t+=deltaT;//t+=SQRT3*ball_radius/velLength;

        pos = ballPos + ballVel*t;
    }
}


//-------------------TESTED-------------------------------
bool InGame::outOfBounds(const GridPoint& gp)
{
    DEBUG_REC();

    if (
        (gp.row <0) ||
         (gp.row >=getWellMatrixHeight() ) ||
         (gp.col<0) ||
         (gp.col > maxBalls(gp.row))
        )
            return true;
    return false;
}
//----------------------------------------------------------
void InGame::emptyCollisionTestMatrix()
{
    DEBUG_REC();

    for ( int row=0; row < getWellMatrixHeight(); row++)
        for ( int col=0;col < WELL_WIDTH;col++)
            collisionTestMatrix[row][col]=false;
}
//-------------------SEEMS OK-------------------------------------
bool InGame::pointAlreadyTested(const GridPoint& gp)
{
    DEBUG_REC();

    if (outOfBounds(gp)) return true;
    return collisionTestMatrix[gp.row][gp.col];
}
//----------------SEEMS OK, SUPERFLOOUS TEST OF VALIDITY, DEBUG LEFT----------------------------------
void InGame::markPointAsTested( const GridPoint& gp)
{
    DEBUG_REC();

    if (outOfBounds(gp)) return;//unnecessary precaution //TO BE REMOVED AFTER TESTING

    collisionTestMatrix[gp.row][gp.col]=true;

}
//--------------------------------------------------------------
bool InGame::testCollisionPoint(const FPoint& pos, const FPoint& vel, const GridPoint& gp, float timeLimit,bool realMode)
{
    DEBUG_REC();

    if (!pointAlreadyTested(gp))
    {
        if (realMode)
            markPointAsTested(gp);
        if (well[gp.row][gp.col]!=NULL)
            return collidesWith(pos,vel, gp,timeLimit);
    }
    return false;
}
//--------------------------------------------------------------

bool InGame::testVertexNeighBors(const FPoint& pos, const FPoint& vel,const GridPoint& gp,float timeLimit, bool realMode)
{
    DEBUG_REC();

    bool found=false;

    if (testCollisionPoint(pos,vel,gp,timeLimit,realMode))
    {
        if (realMode)
            addCollision(pos,vel,gp,timeLimit);
        found=true;
    }

    GridPoint np;

    for (int relativeRow=-1;relativeRow<=1;relativeRow++)
    {
        np.row = gp.row+relativeRow;

        for (int neighbor=NEIGHBOR_LEFT;neighbor<=NEIGHBOR_RIGHT;neighbor++)
        {
            np.col = xIndex(gp, relativeRow, neighbor);

            if ( (np.col<0)||(np.col>=maxBalls(np.row)))
                continue;

            if (testCollisionPoint(pos,vel,np,timeLimit,realMode))
            {
                if (realMode)
                    addCollision(pos,vel,np,timeLimit);
                found=true;
            }

        }
    }
    return found;
}

//---------------------------------------------------------
bool InGame::collidesWith(const FPoint& pos, const FPoint& vel,const GridPoint &gp, float dt)
{
    DEBUG_REC();

    FPoint posAtDt=pos + vel*dt;

    FPoint otherBall(getBallXCoord(gp.row,gp.col),getBallYCoord(gp.row));

    float effective_radius=ball_radius-collision_slack;

   // if (testBallCollision(pos,otherBall,effective_radius,effective_radius))//this is a fault DO NOT ENABLE. PRODUCES ERRORS
    //    return true;

    if (dt==0.) return false;

    if (testBallCollision(posAtDt,otherBall,effective_radius,effective_radius))
        return true;

    FPoint dp=vel*dt;
    FPoint pb = otherBall - pos;

    double t=0;     //((pos + t*dp) -otherpos)*dp =0

    t=(pb*dp)/(dp*dp);//where  the distance is minimal

    if ( (0. <= t) && (t <= 1.))
    {
        FPoint newp=pos + dp*t;

        pb=otherBall -newp;
#ifdef CONSOLE_LOG
            sprintf(glbLogMsg," dist=%.8f ball_rad=%.8f\n",pb.GetLength(),ball_radius);
            mySDL_Log(glbLogMsg);
#endif // CONSOLE_LOG

        if ((pb*pb) <= (4.*effective_radius*effective_radius))
            return true;
    }
    return false;
}
//---------------------------------------------------------
bool InGame::testBallCollision(FPoint b1, FPoint b2, float r1, float r2)
{
    DEBUG_REC();

    float minDistSqr = (r1+r2);
    minDistSqr *= minDistSqr;

    FPoint dp=b2-b1;

    if ( dp*dp <= minDistSqr ) return true;//so >=0 is collision, <0 is not
    return false;
}
//---------------------------------------------------------
void InGame::addCollision(const FPoint& pos, const FPoint& vel,const GridPoint &gp ,float dt)
{
    DEBUG_REC();

    float collisionTime=findCollisionTime(pos,vel,gp );
    if (collisionTime<=dt)
    {
        collisions[numCollisions].gp=gp;
        collisions[numCollisions].type=TYPE_BALL;
        collisions[numCollisions].dt=collisionTime;
        numCollisions++;
#ifdef CONSOLE_LOG
        sprintf(glbLogMsg,"Collision #%d with ( %d, %d )",numCollisions,gp.row,gp.col);
        mySDL_Log(glbLogMsg);
        sprintf(glbLogMsg,"Collision detected in %.6f\n",collisionTime);
        mySDL_Log(glbLogMsg);
#endif // CONSOLE_LOG
    }
    else
    {
#ifdef CONSOLE_LOG
        sprintf(glbLogMsg," Collision found:( %d, %d )\n",gp.row, gp.col);
        mySDL_Log(glbLogMsg);
        sprintf(glbLogMsg," Collision IGNORED: time was %.6f\n",collisionTime);
        mySDL_Log(glbLogMsg);
#endif // CONSOLE_LOG
    }


}
//---------------------------------------------------------
float InGame::findCollisionTime(const FPoint& pos, const FPoint& vel, const GridPoint &gp )
{
    DEBUG_REC();

    float t1,t2;
    double D;
    float effective_radius=ball_radius-collision_slack;


    FPoint dp( getBallXCoord(gp.row,gp.col)-pos.x , getBallYCoord(gp.row)-pos.y);

    double dpv=dp*vel;
    double vsq=vel*vel;

    D =  dpv*dpv  - vsq*( (dp*dp) - (4*effective_radius*effective_radius));//determinant


    if ( D < 0)
        return 200.;


    t1= ( dpv + sqrt(D) ) /vsq;

    t2= ( dpv - sqrt(D) ) /vsq;

#ifdef CONSOLE_LOG
        sprintf(glbLogMsg,"Collision with ( %d, %d )",gp.row,gp.col);
        mySDL_Log(glbLogMsg);
        sprintf(glbLogMsg,"Collision detected in( %.5f )-( %.5f )\n",t1,t2);
        mySDL_Log(glbLogMsg);

#endif // CONSOLE_LOG

    return std::min(t1,t2);//one upon entry , one upon exit
}
//===============EO CLASS FREEBALL=============================
//-------------------------------------------------

GridPoint InGame::findNearestGridPos(FPoint pos)
{
    DEBUG_REC();

    GridPoint pt;

    pt.row = floor(( pos.y - topSurfaceY + ball_radius*(SQRT3/2.-1.)) / (ball_radius*SQRT3));

    if (pt.row<0) pt.row=0;
    else
        if (pt.row > getWellMatrixHeight()-1)
            pt.row =getWellMatrixHeight();

    pt.col = std::min( maxBalls(pt.row)-1,
                      (int)floor((pos.x -(pt.row&1)*ball_radius- rcWell.x) / (ball_radius*2.)) );// &1 was %2
    if (pt.col <0)
        pt.col=0;

    return pt;

}

//---------------------------------------------------------------------

void InGame::initMap()
{
    DEBUG_REC();

    int row,col;
    for ( row=0; row<getWellMatrixHeight(); row++)
        for ( col=0; col<WELL_WIDTH; col++)
        {
            if (well[row][col]==NULL)
                conmap[row][col]=-1;//non existent
            else
                    conmap[row][col]=0;//non visited
        }
}
//---------------------------------------------------------------------------
bool InGame::dropUnconnectedBalls(bool doingLevelGeneration,const GridPoint &dropper)
{
    DEBUG_REC();

    PROFILER_START();
    int row,col;

    int numDroppedBalls=0;

    initMap();

    for (col=0;col<WELL_WIDTH;col++)//traverses map to characterize balls
    {
        if (conmap[0][col]==0)
            DFS(GridPoint(0,col));
    }
    //when this process is finished, all connected balls will have 1 on the map,
    //while unconnected balls or groups will have 0

    for (row=1;row<getWellMatrixHeight();row++)
        for (col=0;col<maxBalls(row);col++)
            if (conmap[row][col]==0)
            {
                if (!doingLevelGeneration)
                {
                    dropBall(GridPoint(row,col),SCORE_DROPPED,dropper);
                    numDroppedBalls++;
                }
                else //called during level generation
                    deleteBall( GridPoint (row,col));
            }
    roundScore+= numDroppedBalls*SCORE_DROPPED;

    PROFILER_STOP();
    return (numDroppedBalls>0);
}
//----------------------------------------------------------------------------
void InGame::deleteBall(const GridPoint& gp)
{
    DEBUG_REC();

    if (well[gp.row][gp.col]!=NULL)
    {
        numStaticBalls--;
        colorStats[ well[gp.row][gp.col]->getColor() ]--;
        delete well[gp.row][gp.col];
        well[gp.row][gp.col]=NULL;
    }
}
//----------------------------------------------------------------------------
void InGame::dropBall(const GridPoint &gp,int scoreType,const GridPoint &dropper)
{
    DEBUG_REC();


#ifdef CONSOLE_LOG
              sprintf(glbLogMsg,"dropping %d, %d\n",gp.row,gp.col);
              mySDL_Log(glbLogMsg);
#endif
    if (well[gp.row][gp.col]!=NULL)
    {
        numStaticBalls--;
        colorStats[ well[gp.row][gp.col]->getColor() ]--;

        well[gp.row][gp.col]->setState(BALLSTATE_FALLING,scoreType);

        if (dropper.row>=0)
        {
            FPoint v(0.,0.);
            v.x = getBallXCoord(gp.row,gp.col)-getBallXCoord(dropper.row,dropper.col);
            v.y = getBallYCoord(gp.row) - getBallYCoord(dropper.row);

            float len=v.GetLength()/4;///ball_radius;

            v*=(BALL_VELOCITY/(len*len));//*ball_radius));
            v+=lastVelocity/10;

            well[gp.row][gp.col]->setVelocity(v);
        }
        else
        {
            FPoint v(0.,0.);
            v.x = genRand10000()-5000;
            v.y = genRand10000()-5000;
            float len=v.GetLength();

            v*=(BALL_VELOCITY/50)/len;
            v+=lastVelocity/10;

            well[gp.row][gp.col]->setVelocity(v);

        }

        well[gp.row][gp.col]->setAcceleration(FPoint(0,
                                                     (1+0.5*(std::max(0.,velocityMultiplier-1.)))*BALL_VELOCITY));

        fallingBalls.push_front(well[gp.row][gp.col]);
        well[gp.row][gp.col]=NULL;
        switch(scoreType)
        {
            case SCORE_BOMB:
            case SCORE_EXPLODED: soundEffect(SFX_EXPLODE);break;
            case SCORE_MATCHED: soundEffect(SFX_POP);break;
            default:soundEffect(SFX_DROP);break;
        }
    }
#ifdef CONSOLE_LOG
        sprintf(glbLogMsg,"....done\n");
        mySDL_Log(glbLogMsg);
#endif
}
//-------------------------------------------------------------------------
void InGame::DFS( const GridPoint &gp )
{
    DEBUG_REC();

    int ncol;
    int nrow;
    conmap[gp.row][gp.col]=1;



    for (int i=-1;i<2;i++)//relative row
    {
        nrow=gp.row+i;

        if ( (nrow >=0) && ( nrow < getWellMatrixHeight() ) )
            for ( int n = NEIGHBOR_LEFT; n <= NEIGHBOR_RIGHT; n++ )//neighbor on row
            {
                ncol=xIndex( gp,i,n);

                if ( (ncol >=0 ) &&( ncol < maxBalls(nrow) ) )
                {
                    if (conmap[nrow][ncol]==0)//non-visited
                        DFS(GridPoint(nrow,ncol));
                }

            }
        }
}

//===============CLASS WELL=============================
//------------------------------------------------
void InGame::resetTopPlate()
{
    DEBUG_REC();

    piston_step=0;
    initial_step=piston_step;

    recs[TEX_WELL_PISTONS]->h = 0;
    recs[TEX_WELL_TOP_PLATE]->y = recs[TEX_WELL_BACKGROUND]->y-recs[TEX_WELL_TOP_PLATE]->h;

    topSurfaceY=recs[TEX_WELL_TOP_PLATE]->y + recs[TEX_WELL_TOP_PLATE]->h;

    recs[TEX_WELL_PISTON_BACKGROUND]->h = topSurfaceY - recs[TEX_WELL_PISTON_BACKGROUND]->y;

    adjustStaticPositions();
}

void InGame::checkTopPlateAction()
{
    DEBUG_REC();

    if ( roundTimer.GetTicks() > ((piston_step-initial_step)+1)*pistonDuration )
    {
        bool ballsCrossed=lowerTopPlate();
        invTexWellHardware=true;

        if (ballsCrossed)
            endLevel(false);
    }
}


bool InGame::lowerTopPlate(int numSteps)
{
    DEBUG_REC();

    if (piston_step <= WELL_HEIGHT-numSteps) piston_step+=numSteps;

    recs[TEX_WELL_PISTONS]->h = getYCoord(piston_step-1)-ball_radius-recs[TEX_WELL_BACKGROUND]->y;

    recs[TEX_WELL_TOP_PLATE]->y =getYCoord(piston_step-1)-ball_radius;

    topSurfaceY=recs[TEX_WELL_TOP_PLATE]->y + recs[TEX_WELL_TOP_PLATE]->h;

    recs[TEX_WELL_PISTON_BACKGROUND]->h = topSurfaceY - recs[TEX_WELL_PISTON_BACKGROUND]->y;


    #ifdef CONSOLE_LOG
        sprintf(glbLogMsg,"lowerTopPlate::piston_step=%d\n",piston_step);
        mySDL_Log(glbLogMsg);
    #endif // CONSOLE_LOG
    return adjustStaticPositions();

}

//------------------------------------------------
bool InGame::raiseTopPlate()//DONE
{
    DEBUG_REC();

    if (piston_step > 0) piston_step--;

    recs[TEX_WELL_PISTONS]->h = getYCoord(piston_step-1)-ball_radius-recs[TEX_WELL_BACKGROUND]->y;

    recs[TEX_WELL_TOP_PLATE]->y =getYCoord(piston_step-1)-ball_radius;

    topSurfaceY=recs[TEX_WELL_TOP_PLATE]->y + recs[TEX_WELL_TOP_PLATE]->h;

    recs[TEX_WELL_PISTON_BACKGROUND]->h = topSurfaceY - recs[TEX_WELL_PISTON_BACKGROUND]->y;

    #ifdef CONSOLE_LOG
        sprintf(glbLogMsg,"raiseTopPlate::piston_step=%d\n",piston_step);
        mySDL_Log(glbLogMsg);
    #endif // CONSOLE_LOG
    return adjustStaticPositions();

}
//--------------------------------------------------------------

bool InGame::adjustStaticPositions()
{
    DEBUG_REC();

    bool lineCrossed=false;

    for (int row=0;row < getWellMatrixHeight();row++)
        for (int col=0;col<maxBalls(row);col++)
        {
            if (well[row][col]!=NULL)
            {
                well[row][col]->setPosition( FPoint( getBallXCoord(row,col),
                                                     getBallYCoord(row)) ) ;

                if (row>=WELL_HEIGHT-piston_step) lineCrossed =true;
#ifdef CONSOLE_LOG
                sprintf(glbLogMsg,"adjusting (%d,%d)\n",row,col);
                mySDL_Log(glbLogMsg);
#endif // CONSOLE_LOG
            }
        }
        return lineCrossed;
}
//==================== EO CLASS WELL ======================

//===================Display funcs============

void InGame::Render(float phase)
{
    DEBUG_REC();
    PROFILER_START();


    //displayBGColor(phase);
   // displayBackground(phase);    //playing space


    if ( (playState!=MODE_PLAY) || (gameSettings->renderBackgroundEffect))
        glbEffect->render();
    else
    {
        int ticks=countDownTimer.GetTicks();
        if (ticks<1000)
            glbEffect->render(1.-ticks/1000.);
    }

    switch (playState)
    {
        case MODE_PLAY:
        case MODE_EDITOR:
        case MODE_PREGAME: gameRender(phase);break;

        case MODE_ENDGAME: gameRender(phase);endGameRender(phase);break;

        case MODE_PAUSE:
        case MODE_QUIT:     if (texPaused!=NULL) texPaused->render(0,0);
                            else gameRender(phase);
                            pauseScreenRender(phase);break;

        case MODE_SUMMON_MENU:if (texPaused!=NULL) texPaused->render(0,0);
                            else gameRender(phase);
                            summonMenuRender(phase);break;

        case MODE_GAME_MENU: gameMenuRender(phase);break;
        case MODE_LEVEL_SELECT: levelSelectorRender(phase);break;
    }

    //SDL_RenderPresent(glbRenderer);//this is called by the state machine to facilitate profiling

    PROFILER_STOP();

}
//-----------------------------------------------

void InGame::gameRender(float phase)
{

    PROFILER_START();

    displayWellBackground(phase);//CAN BE OPTIMIZED IF TRANSP BGS AND BG EFFECT IS OFF

    //the BG drawing version should have init before drawBG
    static bool prevTrails=false;

    bool trails = isEffectActive && gameSettings->renderTrails && (playState==MODE_PLAY);



    if ( trails )
    {
        trailsEffect->initFrame( gameSettings->colors[   ( timeStopActive ? COLOR_TIMESTOP_BACKGROUND : COLOR_WELL_BACKGROUND )   ] );
        if (prevTrails)
            trailsEffect->renderPreviousFrame();
        trailsEffect->excludeFromEffect();
    }

    prevTrails=trails;

    if  ( (playState!=MODE_ENDGAME) && (playState!=MODE_EDITOR)&& (gameSettings->useBallIndex)&&(currentBall->getColor()!=COLOR_CHROME))
        displayBallIndex();


    displayFallingBalls();

    displayStaticBalls();

    if  ( (playState!=MODE_ENDGAME)&& (playState!=MODE_EDITOR) && (gameSettings->useBallIndex)&& (currentBall->getColor()==COLOR_CHROME))//must be drawn above the static balls
        displayBallIndex();

    if (trails)       trailsEffect->includeInEffect();

    displayBallEffects();

if (trails)       trailsEffect->excludeFromEffect();

    if (gameSettings->fullSceneComplexity)
        displayEndLine(phase);
//------------------------------------------------
    PROFILER_ARB_START("InGame::LowBalls");

if (trails) trailsEffect->includeInEffect();

    if (playState!=MODE_EDITOR)
    {
        displayBall(nextBall);
        nextBall->renderEffects();
    }

    displayLauncher();

if (trails && (currentBall->getState()==BALLSTATE_MOVING ))    trailsEffect->excludeFromEffect();

    displayBall(currentBall);
    currentBall->renderEffects(true,false);//shine


if (trails)        trailsEffect->includeInEffect();

    currentBall->renderEffects(false,true);

    PROFILER_ARB_STOP("InGame::LowBalls");
//---------------------------------------------------
    flashEffect->render();

    if (trails)
        trailsEffect->renderFrame(0,0);

    if (playState!=MODE_EDITOR)
        displaySummonIndicator();

    displayWellHardware();

    displayGameInfo();
    displayMessages();
    //displayMatrix();


    PROFILER_STOP();
}
//------------------------------------------------------------------------------

void InGame::displaySummonIndicator()
{
    int summon_radius=recs[TEX_TIMESTOP]->w/2;
    texSummon->render(summonPoint.x - summon_radius,
                          summonPoint.y-summon_radius,
                          recs[TEX_TIMESTOP],-1,summonAlpha);

    if (summonEffect!=NULL)
        summonEffect->render();
}
//------------------------------------------------------------------------------
void InGame::renderStaticObject(int tex,int alpha)//DONE
{
    glbTexManager->getTexturePtr(handles[tex])->render(recs[tex]->x,recs[tex]->y,recs[tex],-1,alpha);
}
//---------------------------------------------------------
void InGame::displayWellBackground(float phase)
{
    int backgroundAlpha =  ( gameSettings->translucentBackgrounds ? (220 + 25*myCosine(phase/3)):255);

    if (playState==MODE_EDITOR)
        backgroundAlpha=255;

    if (!timeStopActive)
        renderStaticObject(TEX_WELL_BACKGROUND,backgroundAlpha);
    else
    {
        SDL_SetRenderDrawColor(glbRenderer, gameSettings->colors[COLOR_TIMESTOP_BACKGROUND].r,
                                            gameSettings->colors[COLOR_TIMESTOP_BACKGROUND].g,
                                            gameSettings->colors[COLOR_TIMESTOP_BACKGROUND].b,
                                            backgroundAlpha);
        SDL_RenderFillRect(glbRenderer,&rcWell);
    }
}
//---------------------------------------------------------
void InGame::renderOffsetStaticObject(int tex, const FPoint& offset,int alpha)
{
    glbTexManager->getTexturePtr(handles[tex])->render(recs[tex]->x+offset.x,recs[tex]->y+offset.y,recs[tex],-1,alpha);
}
//---------------------------------------------------------
void InGame::updateWellHardwareImage()
{
    DEBUG_REC();

    PROFILER_START();

    if (texWellHardware==NULL)
    {
        texWellHardware= new BaseTexture;

        if (!texWellHardware->createBlank( rcBorder.w,rcBorder.h,SDL_TEXTUREACCESS_TARGET) )
        {
    #ifdef CONSOLE_LOG
            sprintf(glbLogMsg, "InGame:texWellHardwareImage creation Failed\n");
            mySDL_Log(glbLogMsg);
    #endif // CONSOLE_LOG

            delete texWellHardware;
            texWellHardware=NULL;
        }
        else
            texWellHardware->useAlphaBlending(true);
    }

    if (texWellHardware!=NULL)
    {
        texWellHardware->setAsRenderTarget();

        FPoint offset( -rcBorder.x, -rcBorder.y );

        renderOffsetStaticObject(TEX_WELL_LEFT,offset);
        renderOffsetStaticObject(TEX_WELL_RIGHT,offset);
        renderOffsetStaticObject(TEX_WELL_TOP,offset);
        renderOffsetStaticObject(TEX_WELL_BOTTOM,offset);

        displayWellCorners(offset);

        if (piston_step>0)
            renderOffsetStaticObject(TEX_WELL_TOP_PLATE,offset);
        if (piston_step>1)
            renderOffsetStaticObject(TEX_WELL_PISTONS,offset);

        renderLevelStats(offset);

        texWellHardware->resetRenderTarget();

    }
    invTexWellHardware=false;

    PROFILER_STOP();
}
//--------------------------------------------------------

void InGame::displayWellCorners( const FPoint& offset)
{
    renderOffsetStaticObject(TEX_WELL_LCORNER,offset);
    renderOffsetStaticObject(TEX_WELL_RCORNER,offset);

    glbTexManager->getTexturePtr(handles[TEX_WELL_LCORNER])->render( recs[TEX_WELL_LCORNER]->x+offset.x,
                                                        recs[TEX_WELL_BOTTOM]->y+offset.y,
                                                        recs[TEX_WELL_LCORNER],-1,255,0.,NULL,
                                                        SDL_FLIP_VERTICAL
                                                        );
    glbTexManager->getTexturePtr(handles[TEX_WELL_RCORNER])->render( recs[TEX_WELL_RCORNER]->x+offset.x,
                                                        recs[TEX_WELL_BOTTOM]->y+offset.y,
                                                        recs[TEX_WELL_RCORNER],-1,255,0.,NULL,
                                                        SDL_FLIP_VERTICAL
                                                        );
}

//------------------------------------------------------------------------------------

void InGame::displayWellHardware()
{
    PROFILER_START();
    if (piston_step>0)
       displayWellCompressorBackground();

    if (texWellHardware!=NULL)
        texWellHardware->render(rcBorder.x,rcBorder.y);
    else
    {
        displayWellSides();
        displayWellCorners();
        displayWellCompressor();
    }
    PROFILER_STOP();
}
//---------------------------------------------------------
void InGame::displayWellSides()
{
    renderStaticObject(TEX_WELL_LEFT);
    renderStaticObject(TEX_WELL_RIGHT);
    renderStaticObject(TEX_WELL_TOP);
    renderStaticObject(TEX_WELL_BOTTOM);
}
//--------------------------------------------------------
void InGame::displayWellCompressorBackground()
{
      int bgAlpha = 200*(roundTimer.GetTicks()*1./pistonDuration - 1.*(piston_step-initial_step));

       if (bgAlpha>255) bgAlpha=255;

       renderStaticObject(TEX_WELL_PISTON_BACKGROUND,bgAlpha);
}
//---------------------------------------------------------
void InGame::displayWellCompressor()
{
   if (piston_step>0)
       renderStaticObject(TEX_WELL_TOP_PLATE);
   if (piston_step>1)
        renderStaticObject(TEX_WELL_PISTONS);
}
//---------------------------------------------------------
void InGame::displayEndLine(float phase)
{
    PROFILER_START();
    renderStaticObject(TEX_WELL_ENDLINE,(int)(100 + 60*myCosine(phase*(gameSettings->pressure)*3)));
    PROFILER_STOP();
}
//------------------------------------------------------------
void InGame::displayApproximateGridPos(FPoint realPos)
{
        GridPoint pt=findNearestGridPos(realPos);

        FPoint p( getBallXCoord(pt.row,pt.col), getBallYCoord(pt.row) );

        glbTexManager->getTexturePtr(handles[COLOR_YELLOW])->render(p.x-ball_radius,
                                                                          p.y-ball_radius,
                                                                          recs[COLOR_PURPLE], -1,150);
}

void InGame::updateBallIndexImage()
{
    DEBUG_REC();

    PROFILER_START();


    if (texBallIndex==NULL)
    {
        texBallIndex= new BaseTexture;
        if (!texBallIndex->createBlank( rcWell.w,rcWell.h,SDL_TEXTUREACCESS_TARGET) )
        {
    #ifdef CONSOLE_LOG
            sprintf(glbLogMsg, "InGame:texWellHardwareImage creation Failed\n");
            mySDL_Log(glbLogMsg);
    #endif // CONSOLE_LOG

            delete texBallIndex;
            texBallIndex=NULL;
        }
        else texBallIndex->useAlphaBlending(true);
    }

    if (texBallIndex!=NULL)
    {
            texBallIndex->setAsRenderTarget(gameSettings->colors[ (timeStopActive ? COLOR_TIMESTOP_BACKGROUND : COLOR_WELL_BACKGROUND) ]);

            FPoint offset( -rcWell.x, -rcWell.y );

            renderBallIndex(offset);//,true,false);

            texBallIndex->resetRenderTarget();
    }
    invTexBallIndex=false;

    PROFILER_STOP();

}
//---------------------------------------------------------
void InGame::displayBallIndex( )
{
PROFILER_START();
    if (texBallIndex!=NULL)
        texBallIndex->render(rcWell.x,rcWell.y,NULL,-1,190);//(gameSettings->renderTrails)?65:190);
    else
        renderBallIndex();
PROFILER_STOP();
}

//---------------------------------------------------------
void InGame::renderBallIndex(const FPoint& offset)//,bool renderPath, bool renderGhost)
{

PROFILER_START();

    int alpha=190;//(texBallIndex!=NULL)?255:190;

    int length = indexCollides? indexLength-1: indexLength;

    for (int i=0; i<length ;i++)
        glbTexManager->getTexturePtr( handles[ currentBall->getColor() ] )->render(
                                                offset.x+ballIndex[i].x -recs[TEX_BALL_INDEX]->w/2.,
                                                offset.y+ballIndex[i].y -recs[TEX_BALL_INDEX]->h/2.,
                                                recs[TEX_BALL_INDEX],-1,alpha);

    if (indexCollides)
    {
/*        if(texBallIndex!=NULL)//that is beecause of the render to buffer setting in texture.cpp
            alpha=(gameSettings->skin==SKIN_DARK)?165:130;
        else*/
            alpha=((gameSettings->skin==SKIN_DARK)  || (gameSettings->skin==SKIN_DARK2))?80:60;//65:40;

        glbTexManager->getTexturePtr( handles[ currentBall->getColor() ] )->render(
                                                    offset.x+ballIndex[indexLength-1].x -ball_radius,//recs[COLOR_GREEN]->w/2.,
                                                    offset.y+ballIndex[indexLength-1].y -ball_radius,//recs[COLOR_GREEN]->h/2.,
                                                    recs[COLOR_GREEN],-1,alpha
                                                    );
    }
PROFILER_STOP();
}

//---------------------------------------------------------
void InGame::renderLevelStats(const FPoint& offset)
{
    int ypos=offset.y+recs[TEX_WELL_ENDLINE]->y+ (launchPoint.y-recs[TEX_WELL_ENDLINE]->y - texRoundScore[STAT_BEST]->getHeight())/2;//ball_radius/4;

    if (gameSettings->showRoundScore)
    {
        texRoundScore[STAT_LEGEND]->render(offset.x+(rcWell.x+launchPoint.x)/2 - texRoundScore[STAT_LEGEND]->getWidth(),
                                            ypos+(texRoundScore[STAT_CURRENT]->getHeight()-texRoundScore[STAT_LEGEND]->getHeight())/2,NULL,-1,160);

        texRoundScore[STAT_CURRENT]->render(offset.x+recs[TEX_WELL_ENDLINE]->x + ball_radius/4,    ypos);

        if (gameSettings->showBest)
            texRoundScore[STAT_BEST]->render( offset.x+(recs[TEX_WELL_ENDLINE]->x + launchPoint.x)/2,    ypos);
    }
    //else
      //  texScore->render(offset.x+recs[TEX_WELL_ENDLINE]->x + ball_radius/4,   ypos);

    texLevel->render(offset.x+launchPoint.x + 2.5*ball_radius,   ypos);

    if (gameSettings->showRoundTime)
    {
        texRoundTime[STAT_LEGEND]->render(offset.x+rcWell.x+rcWell.w/2 - texRoundTime[STAT_LEGEND]->getWidth()/2,
                            offset.y+rcWell.y-ball_radius/8-texRoundTime[STAT_LEGEND]->getHeight(),NULL,-1,160);

        if (gameSettings->showBest)
            texRoundTime[STAT_BEST]->render(offset.x+rcWell.x + rcWell.w/2 +2*ball_radius,
                            offset.y+rcWell.y-ball_radius/8-texRoundTime[STAT_BEST]->getHeight());
    }

    ypos=offset.y+ rcWell.y+rcWell.h+ball_radius/8;

    if (gameSettings->showRoundShots)
    {
        texRoundShots[STAT_LEGEND]->render(offset.x + rcWell.x + rcWell.w/2 -texRoundShots[STAT_LEGEND]->getWidth()/2,  ypos,NULL,-1,160);

        texRoundShots[STAT_CURRENT]->render(offset.x + rcWell.x + rcWell.w/2 -texRoundShots[STAT_CURRENT]->getWidth()-2*ball_radius,    ypos);

        if (gameSettings->showBest)
            texRoundShots[STAT_BEST]->render(offset.x+rcWell.x + rcWell.w/2 +2*ball_radius,   ypos);
    }

}
//---------------------------------------------
void InGame::displayGameInfo()
{
PROFILER_START();

    if (texWellHardware==NULL)
        renderLevelStats();

    if (gameSettings->showRoundTime)
    {
        texRoundTime[STAT_CURRENT]->render(rcWell.x + rcWell.w/2 -std::max(texRoundTime[STAT_CURRENT]->getWidth(),(int)(3.5*ball_radius)),
                            rcWell.y-ball_radius/8-texRoundTime[STAT_CURRENT]->getHeight());
    }

PROFILER_STOP();
}

//--------------------------------------------------
void InGame::displayMessages()
{
    PROFILER_START();

    if (playState==MODE_PREGAME)
    {
        texMessage->render(  (screenRect->w - texMessage->getWidth())/2,
                                0.33*guiBox->h );

        texCountDown->render((screenRect->w - texCountDown->getWidth())/2,
                                0.66*guiBox->h );
    }
    else
        if (playState==MODE_PLAY)
        {
            int ticks=countDownTimer.GetTicks();
            if (ticks<1000)
               texMessage->render(  (screenRect->w - texMessage->getWidth())/2,
                                0.33*guiBox->h ,NULL,-1,255*(1.-ticks/1000.));

        }

    PROFILER_STOP();
}
//---------------------------------------------------------
void InGame::displayMatrix()
{
    PROFILER_START();
    PROFILER_STOP();

    FPoint pt;
    int color;
    for (int row=0;row < getWellMatrixHeight()-piston_step;row++)
        for (int col=0;col<maxBalls(row);col++)
        {
           // if (well[row][col]!=NULL)
           //     continue;

            if ( pointAlreadyTested(GridPoint(row,col)) )
                color=COLOR_PURPLE;
            else
                color=COLOR_BLACK;

            pt=FPoint(getBallXCoord(row,col),getBallYCoord(row));

            glbTexManager->getTexturePtr(handles[color])->render( pt.x -recs[TEX_BALL_INDEX]->w/2.,
                                                    pt.y -recs[TEX_BALL_INDEX]->h/2.,
                                                    recs[TEX_BALL_INDEX],-1,190);

        }

}
//---------------------------------------------------------
void InGame::updateStaticBallImage()
{
    DEBUG_REC();

    PROFILER_START();

    if (texStaticBalls!=NULL)
    {
        if (texStaticBalls->getHeight()!=(rcWell.h -(topSurfaceY-rcWell.y -ball_radius*SQRT3)))
        {
            delete texStaticBalls;
            texStaticBalls=NULL;
        }
    }

    if (texStaticBalls==NULL)
    {
        texStaticBalls= new BaseTexture;

        if (!texStaticBalls->createBlank( rcWell.w,rcWell.h -(topSurfaceY-rcWell.y-ball_radius*SQRT3),
                                           SDL_TEXTUREACCESS_TARGET) )
        {
    #ifdef CONSOLE_LOG
            sprintf(glbLogMsg, "InGame:StaticBallImage creation Failed\n");
            mySDL_Log(glbLogMsg);
    #endif // CONSOLE_LOG

            delete texStaticBalls;
            texStaticBalls=NULL;
        }
        else
            texStaticBalls->useAlphaBlending(true);
    }

    if (texStaticBalls!=NULL)
    {
        texStaticBalls->setAsRenderTarget();

        FPoint offset(-rcWell.x,-topSurfaceY);

        for (int row=0; row<getWellMatrixHeight(); row++)
            for ( int col=0; col<maxBalls(row); col++)
            {
                if ( well[row][col] != NULL)
                        well[row][col]->renderBall( handles[ well[row][col]->getColor()],
                                                    *recs[COLOR_GREEN],offset);
            }

        texStaticBalls->resetRenderTarget();
    }

    invTexStatic=false;

    PROFILER_STOP();
}
//---------------------------------------------------------
void InGame::displayStaticBalls()
{
PROFILER_START();

    if (texShiningBalls!=NULL)
        texShiningBalls->render(rcWell.x,topSurfaceY);
    else if (texStaticBalls!=NULL)
            texStaticBalls->render(rcWell.x,topSurfaceY);
    else
    {
        for (int row=0; row<getWellMatrixHeight(); row++)
            for ( int col=0; col<maxBalls(row); col++)
            {
                if ( well[row][col] != NULL)
                        displayBall(well[row][col]);
            }
    }

PROFILER_STOP();
}
//-----------------------------------------------------------
void InGame::updateShineEffectImage()
{
    DEBUG_REC();

    if (!gameSettings->useShineEffect)
        return;
    PROFILER_START();


    if (texShiningBalls!=NULL)
    {
        if (texShiningBalls->getHeight() != (rcWell.h -(topSurfaceY-rcWell.y)-ball_radius*SQRT3) )
        {
            delete texShiningBalls;
            texShiningBalls=NULL;
        }
    }

    if (texShiningBalls==NULL)
    {
        texShiningBalls= new BaseTexture;

        if (!texShiningBalls->createBlank( rcWell.w, rcWell.h -(topSurfaceY-rcWell.y)-ball_radius*SQRT3,
                                        SDL_TEXTUREACCESS_TARGET) )
        {
    #ifdef CONSOLE_LOG
            sprintf(glbLogMsg, "InGame:ShiningBallImage creation Failed\n");
            mySDL_Log(glbLogMsg);
    #endif // CONSOLE_LOG
            delete texShiningBalls;
            texShiningBalls=NULL;
        }
        else
        {
            texShiningBalls->useAlphaBlending(true);
        }
    }

    if (texShiningBalls!=NULL)
    {
        texShiningBalls->setAsRenderTarget();

        texStaticBalls->render(0,0);

        FPoint offset(-rcWell.x,-topSurfaceY);

        for (int row=0; row<getWellMatrixHeight(); row++)
            for ( int col=0; col<maxBalls(row); col++)
            {
                if ( well[row][col] != NULL)
                        well[row][col]->renderEffects(true,false,offset);
            }
        texShiningBalls->resetRenderTarget();
    }

    invTexShining=false;

    PROFILER_STOP();
}
//-----------------------------------------------------------
void InGame::displayBallEffects()
{
    DEBUG_REC();
    PROFILER_START();

    BallInfo* ball=NULL;

    bool renderShine = gameSettings->useShineEffect;

    if (texShiningBalls!=NULL)
    {
        renderShine=false;
    }

    for (int row=0; row < getWellMatrixHeight(); row++)
        for ( int col=0; col<maxBalls(row); col++)
        {
            ball = well[row][col];
            if ( ball != NULL)
                ball->renderEffects(renderShine,true);//here shine
        }


    std::list<BallInfo*>::const_iterator i;
    for( i= fallingBalls.begin(); i != fallingBalls.end();i++)
    {
        ball=*i;
        if (ball ==NULL ) continue;
        else ball->renderEffects();
    }

    PROFILER_STOP();
}
//---------------------------------------------------------
void InGame::displayFallingBalls()
{
    PROFILER_START();

    std::list<BallInfo*>::const_iterator i;
    for( i= fallingBalls.begin(); i != fallingBalls.end();i++)
        if (*i !=NULL )
            displayBall(*i);

    PROFILER_STOP();
}
//---------------------------------------------------------

void InGame::displayBall(BallInfo* ball)//for current and next balls
{
    ball->renderBall(handles[ball->getColor()],
                     *recs[COLOR_GREEN] );
}
//---------------------------------------------------------
void InGame::updatePausedImage()
{
    PROFILER_START();

    if (texPaused==NULL)
    {
        texPaused= new BaseTexture;
        if (!texPaused->createBlank(screenRect->w,screenRect->h,SDL_TEXTUREACCESS_TARGET))
        {

#ifdef CONSOLE_LOG
            sprintf(glbLogMsg, "InGame:LauncherImage creation Failed\n");
            mySDL_Log(glbLogMsg);
#endif // CONSOLE_LOG
            delete texPaused;
            texPaused=NULL;
        }
        else
            texPaused->useAlphaBlending(true);
    }

    if (texPaused!=NULL)
    {
        texPaused->setAsRenderTarget();

        gameRender();

        texPaused->resetRenderTarget();
    }

    PROFILER_STOP();

}


//---------------------------------------------------------
void InGame::updateLauncherImage()
{
    PROFILER_START();
    SDL_Rect rcLauncher={rcWell.x,std::min( (int)(launchPoint.y-recs[TEX_CANNON]->h/2.),recs[TEX_WELL_ENDLINE]->y),rcWell.w,0};

    rcLauncher.h=rcWell.y+rcWell.h-rcLauncher.y;

    if (texLauncher==NULL)
    {
        texLauncher= new BaseTexture;
        if (!texLauncher->createBlank(rcLauncher.w,rcLauncher.h,//rcWell.w, (recs[TEX_MACHINE]->h+recs[TEX_CANNON]->h)/2 ,
                                        SDL_TEXTUREACCESS_TARGET))
        {

#ifdef CONSOLE_LOG
            sprintf(glbLogMsg, "InGame:LauncherImage creation Failed\n");
            mySDL_Log(glbLogMsg);
#endif // CONSOLE_LOG
            delete texLauncher;
            texLauncher=NULL;
        }
        else
            texLauncher->useAlphaBlending(true);
    }

    if (texLauncher!=NULL)
    {
        texLauncher->setAsRenderTarget(gameSettings->colors[ (timeStopActive ? COLOR_TIMESTOP_BACKGROUND : COLOR_WELL_BACKGROUND) ]);

        FPoint offset(-rcLauncher.x,-rcLauncher.y);//-rcWell.x,-(recs[TEX_MACHINE]->y+(recs[TEX_MACHINE]->h-recs[TEX_CANNON]->h)/2));

        if (!gameSettings->fullSceneComplexity)
            renderOffsetStaticObject(TEX_WELL_ENDLINE,offset,200);//HERE

        if (playState==MODE_EDITOR)
        {
            displayPalette(offset);
        }
        else
        {
            renderOffsetStaticObject(TEX_MACHINE,offset);
            glbTexManager->getTexturePtr(handles[TEX_CANNON])->render(recs[TEX_CANNON]->x+offset.x,recs[TEX_CANNON]->y+offset.y,recs[TEX_CANNON],-1,255,angle);
        }
        texLauncher->resetRenderTarget();
    }

    invTexLauncher=false;

    PROFILER_STOP();

}
//---------------------------------------------------------
void InGame::displayPalette(const FPoint& offset)
{
    const int rad=guiUnit/2;

    const SDL_Rect rcPalette={0,0,rad*2,rad*2};

    for (int i=0;i<NUM_BALLCOLORS-NUM_SPECIALBALLS+2;i++)
    {
        glbTexManager->getTexturePtr(handles[palette[i].second])->render(palette[i].first.x + offset.x - rad , palette[i].first.y + offset.y - rad, &rcPalette);
    }
}
//---------------------------------------------------------
void InGame::displayLauncher()
{
    PROFILER_START();

    if (texLauncher!=NULL)
        texLauncher->render(rcWell.x, std::min( (int)(launchPoint.y-recs[TEX_CANNON]->h/2.),recs[TEX_WELL_ENDLINE]->y)  );
    else
    {
        if (!gameSettings->fullSceneComplexity)
            displayEndLine(0);

        if (playState==MODE_EDITOR)
        {
            displayPalette();
        }
        else
        {
            renderStaticObject(TEX_MACHINE);
            glbTexManager->getTexturePtr(handles[TEX_CANNON])->render(recs[TEX_CANNON]->x,recs[TEX_CANNON]->y,recs[TEX_CANNON],-1,255,angle);
        }
    }
    PROFILER_STOP();
}

//friends--------------------------------------------------------------------------
void InGame::renderRawLevel(char *buffer)
{
    DEBUG_REC();

    PROFILER_START();

    renderStaticObject(TEX_WELL_BACKGROUND,220);

    if (buffer!=NULL)
    {
        int step=buffer[0];

        recs[TEX_WELL_PISTONS]->h = (step==0)?0:
                                        getYCoord(step-1)-ball_radius-recs[TEX_WELL_BACKGROUND]->y;
        recs[TEX_WELL_TOP_PLATE]->y = (step==0)?
                                        recs[TEX_WELL_BACKGROUND]->y-recs[TEX_WELL_TOP_PLATE]->h:
                                        getYCoord(step-1)-ball_radius;

        topSurfaceY=recs[TEX_WELL_TOP_PLATE]->y + recs[TEX_WELL_TOP_PLATE]->h;

        int bi=1;
        for (int row=0;row<WELL_HEIGHT;row++)
        {
            int rowLength=maxBalls(row);
            for (int col=0;col<WELL_WIDTH;col++)
            {
                if ( (col<rowLength) && (buffer[bi]>0) )
                {
                    glbTexManager->getTexturePtr(handles[buffer[bi]-1])->render(
                                                                        getBallXCoord(row,col)-ball_radius,
                                                                        getBallYCoord(row)-ball_radius,
                                                                        recs[COLOR_GREEN]);
                }
                bi++;
            }
        }
        if (step>0) renderStaticObject(TEX_WELL_TOP_PLATE);
        if (step>1) renderStaticObject(TEX_WELL_PISTONS);
    }

    displayWellSides();

    displayWellCorners();

  //  renderStaticObject(TEX_WELL_ENDLINE);
  PROFILER_STOP();
}

//--------------------------------------------------------------------

bool InGame::loadRawLevel(const char* filename,char* buffer,bool selectorMode,int level)
{
    DEBUG_REC();

    PROFILER_START();

    for (int i=0;i<WELL_HEIGHT*WELL_WIDTH+1;i++)
        buffer[i]=0;

    SDL_RWops *in=NULL;
    in =SDL_RWFromFile(filename,"r");
    if (!in)
    {
#ifdef RANDOM_LEVELS
        if ( ( isLegitimateCopy )&& (selectorMode)&&(level>=0))
        {
            generateLevel(level,selectorMode);
            makeBufferFromLevel(buffer);
            PROFILER_STOP();
            return true;
        }
#endif // RANDOM_LEVELS


#ifdef CONSOLE_LOG
        sprintf(glbLogMsg,"ERROR READING %s\n",filename);
        mySDL_Log(glbLogMsg);
#endif
        PROFILER_STOP();
        return false;
    }


    SDL_RWread( in, buffer, sizeof(char), WELL_HEIGHT*WELL_WIDTH+1 );

    SDL_RWclose(in);
#ifdef CONSOLE_LOG
        sprintf(glbLogMsg,"read %s\n",filename);
        mySDL_Log(glbLogMsg);
#endif
    PROFILER_STOP();
return true;
}

//------------------------------------------------------------

void InGame::saveRawLevel( const char* filename, char* buffer)
{
    DEBUG_REC();

    SDL_RWops *out=NULL;
    out =SDL_RWFromFile(filename,"w");

    if (!out)
    {
#ifdef CONSOLE_LOG
        sprintf(glbLogMsg,"ERROR CREATING %s\n",filename);
        mySDL_Log(glbLogMsg);
#endif
        return;
    }
    SDL_RWwrite( out, buffer, sizeof(char),WELL_HEIGHT*WELL_WIDTH +1);
    SDL_RWclose(out);

#ifdef CONSOLE_LOG
    sprintf(glbLogMsg,"saved %s\n",filename);
    mySDL_Log(glbLogMsg);
#endif
}
//============helper funcs=============
//-------------------------------------------------------
//(-1..1,RIGHT-LEFT)
int InGame::xIndex(const GridPoint &gp, int relativeRow, int neighbor)
{
    if (relativeRow==0) return (neighbor==NEIGHBOR_LEFT? gp.col-1:gp.col+1);

    return gp.col + neighbor -(! (gp.row&1));
}
//--------------------------------------------------------
int InGame::maxBalls(int row)//per row
{
    return WELL_WIDTH- (row&1);//row%2 //odd rows have one ball less
}
//--------------------------------------------------------

float InGame::getYCoord(int row )
{
    return ( rcWell.y + ball_radius*(1+row*SQRT3));
}
//-------------------------------------------------------
float InGame::getBallXCoord(int row ,int column)
{
    return rcWell.x + ball_radius*( 1+(row&1)+ (column<<1) ); //( 1+(row%2)+2*column );
}

//-------------------------------------------------------
float InGame::getBallYCoord( int row )
{
    return  topSurfaceY + ball_radius*(1+row*SQRT3);
}
//--------------------------------------------------------
int InGame::getWellMatrixHeight()
{
    return WELL_HEIGHT+2;
}
//---------------------------------------------------------------------
void InGame::resetEffectFlags()
{
    for (int i=0; i<NUM_SOUND_EFFECTS;i++)
        soundPlaying[i]=false;

}
//---------------------------------------------------------------------------

void InGame::soundEffect(int effect)
{
    int extras=0;

    if ((effect==SFX_LAND)||(effect==SFX_COLLISION)||(effect==SFX_LAUNCH))
        extras=2;
    else
        if ((effect==SFX_DROP)|| (effect==SFX_EXPLODE))
            extras=4;
        else if (effect==SFX_POP)
            extras=7;

    if (extras == 0)
    {
        if (!soundPlaying[effect])
        {
            playSound(effect,0);
            soundPlaying[effect]=true;
        }
    }
    else
    {
        int start=rand()%extras;
        int step=1+rand()%extras;
        int e;
        for (int i=0; i<extras;i++)
        {
            e=effect + (start+step*i)%extras;
            if (!soundPlaying[e])
            {
                playSound(e,0);
                soundPlaying[e]=true;
                break;
            }
        }
    }
}

//=========GAME MENU=====================================================
void InGame::gameMenuLogic(float dt)
{
    DEBUG_REC();

    int level,action;

    if (gameMenu->isDone(&action,&level))
    {

        delete gameMenu;
        gameMenu=NULL;

        if (action==ACTION_SELECTLEVEL)
        {
           levelSelectorInit(level);
        }
        else
        {
            playState=MODE_PLAY;
            initGame(action,level);
        }
    }
    else
        gameMenu->logic(dt);
}
//------------------------------------------------------------------------
void InGame::gameMenuRender(float phase)
{
    gameMenu->render(phase);
}
//------------------------------------------------------------------------
void InGame::gameMenuHandleEvents(SDL_Event& event)
{
    gameMenu->handleEvents(event);
}
//------------------------------------------------------------------------
void InGame::gameMenuInit()
{
    DEBUG_REC();

    LevelButton::setGameObject(this);
    playState = MODE_GAME_MENU;

    SDL_Rect rc={ 0, 0, (int)((guiBox->w - 0.5*guiUnit)/2.), (int)( (guiBox->h - 1.5*guiUnit)/2.) };

    computeDimensions(&rc);

    gameMenu = new GameMenu();
        //a quarter of the well-shaped screen area

}
//-----------------------------------------------------------------------

//===========================LEVEL SELECTION MODE =================================

void InGame::levelSelectorLogic(float dt)
{
    int level,action;

    if (levelSelector->isDone(&action,&level))
    {

        delete levelSelector;
        levelSelector=NULL;

        playState=MODE_PLAY;
        initGame(action,level);
    }
    else
        levelSelector->logic(dt);
}
//------------------------------------------------------------------------
void InGame::levelSelectorRender(float phase)
{
    levelSelector->render(phase);
}
//------------------------------------------------------------------------
void InGame::levelSelectorHandleEvents(SDL_Event& event)
{
    levelSelector->handleEvents(event);
}
//-----------------------------------------------------------------------

void InGame::levelSelectorInit(int level)
{
    playState = MODE_LEVEL_SELECT;
    //float offset=0.5;

    SDL_Rect rc={ 0, 0, (int)( (guiBox->w-0.45*guiUnit)/3.), (int)((guiBox->h-0.45*guiUnit)/3.)};

    computeDimensions(&rc);

    levelSelector = new LevelSelector(level);
        //a quarter of the well-shaped screen area
}

//--------------------------------------------------------------------------

void InGame::endGameLogic(float dt)
{
    bool playTheLevel;

    if (endGame->hitContinue())
    {
        playState=MODE_PLAY;
        stopTimers();
        initRound();
        pauseTimers();
        playState = MODE_ENDGAME;
    }

    if ( endGame->isDone(&playTheLevel) )
    {

        delete endGame;
        endGame=NULL;

        if (playTheLevel)
        {
            playState=MODE_PREGAME;
            resumeTimers();
        }
        else
        {
            emptyWell();
            levelSelectorInit(0);
        }
    }
    else
        endGame->logic(dt);
}


//---------------------------------------------------------------------------

void InGame::endGameRender(float phase)
{
    endGame->render(phase);

}

//---------------------------------------------------------------------------

void InGame::endGameHandleEvents( SDL_Event& event)
{
    endGame->handleEvents(event);
}

//--------------------------------------------------------------------------

void InGame::pauseScreenLogic(float dt)
{
    bool playOn;

    if ( pauseScreen->isDone(&playOn) )
    {

        delete pauseScreen;
        pauseScreen=NULL;

        if (playOn)
        {
            pauseGame(playState);
        }
        else
        {
            quitSequence();
        }
    }
}


//---------------------------------------------------------------------------

void InGame::pauseScreenRender(float phase)
{
    pauseScreen->render(phase);

}

//---------------------------------------------------------------------------

void InGame::pauseScreenHandleEvents( SDL_Event& event)
{
    pauseScreen->handleEvents(event);
}
//--------------------------------------------------------------------------
void InGame::activateTimeStop()
{
    roundTimer.Pause();
    shotTimer.Pause();

    timeStopActive=true;
    timeStopUsed=true;
    flashEffect->shine(0.3,&rcWell);
#ifdef CONSOLE_LOG
        sprintf(glbLogMsg,"TIMESTOP IS ACTIVE");
        mySDL_Log(glbLogMsg);
#endif // CONSOL
}
//--------------------------------------------------------------------------
void InGame::deactivateTimeStop()
{
    //do not call resume tumers. this will be called through slotNextBall
    timeStopActive=false;
    flashEffect->shine(0.3,&rcWell);
#ifdef CONSOLE_LOG
        sprintf(glbLogMsg,"TIMESTOP DEACTIVATED");
        mySDL_Log(glbLogMsg);
#endif // CONSOL

}

//--------------------------------------------------------------------------
void InGame::summonMenuInit()
{



    pauseTimers();

    playState = MODE_SUMMON_MENU;

    updatePausedImage();

    summonMenu = new SummonMenu(rcWell,gameLevel, timeStopUsed, ballSummoned, ballToSummon, powerBallGenerated );

}
//--------------------------------------------------------------------------
void InGame::summonMenuLogic(float dt)
{

    int summonFeature=0;
    bool commitChanges=false;

    if ( summonMenu->isDone(&commitChanges,&summonFeature) )
    {

        delete summonMenu;
        summonMenu=NULL;

        if (commitChanges)
        {
            if ((summonFeature==UNL_TIMESTOP)&&(!timeStopUsed))
            {
                activateTimeStop();
                if (!ballSummoned)
                    ballToSummon=UNL_NONE;
            }
            else if (!ballSummoned)//if already summoned, I want to keep the summoning until i register it in playerSettings
            {
                ballToSummon=summonFeature;
                if (ballToSummon!=UNL_NONE)
                    flashEffect->shine(0.3,&rcWell);
            }
            setSummonIndicator();
        }

        playState=MODE_PLAY;
        if (!timeStopActive)
            resumeTimers();

    }
    else
        summonMenu->logic(dt);
}

//---------------------------------------------------------------------------
void InGame::summonMenuRender(float phase)
{
    summonMenu->render(phase);
}

//---------------------------------------------------------------------------

void InGame::summonMenuHandleEvents( SDL_Event& event)
{
    summonMenu->handleEvents(event);
}
