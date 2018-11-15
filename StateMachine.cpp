#include "base.h"
#include "globals.h"
#include "timing.h"

#include "stateMachine.h"
#include "states.h"
#include "title.h"
#include "optionsMenu.h"
#include "profileMenu.h"
#include "inGame.h"
#include "aboutScreen.h"
#include "progressInfo.h"

#include "screenEffects.h"
#include "basefuncs.h"
#include "gameSettings.h"
#include "myMath.h"

#include "texture.h"
#include "antihack.h"

#include <cstdio>

//#define FPS_STATS

extern int refreshRate;

extern bool integrityCheckComplete;
extern bool licenceCheckComplete;
extern bool isOriginal;


float framesPerSec;
bool isHibernating;

//-----------------------------------------------

int StateMachine::nextState = STATE_NULL;       //static member variable

//-----------------------------------------------

StateMachine::StateMachine()
{
    currentState = new Entry();
    stateID = STATE_NULL;

    loadImages();
}

//-----------------------------------------------

StateMachine::~StateMachine()
{
    delete currentState;
    currentState=NULL;

    texBackground=NULL;

}
//------------------------------------------------
void StateMachine::loadImages()
{
    texBackground= glbTexManager->getTexturePtr( glbTexManager->insertTexture("background.png",gameSettings->skin,true));
    texBackground->useColorMod(gameSettings->translucentBackgrounds);

}
//-----------------------------------------------

void StateMachine::SetNextState(int newState)
{
     if (nextState!=STATE_EXIT)
        nextState=newState;
}

//-----------------------------------------------------

void StateMachine::ChangeState()
{
    static int oldSkin=gameSettings->skin;

    if (nextState!=STATE_NULL)
    {
#ifdef CONSOLE_LOG
        sprintf(glbLogMsg," Changing State\n");
        mySDL_Log(glbLogMsg);
#endif
        if (nextState!=STATE_EXIT)
        {
            delete currentState;
            currentState=NULL;
        }

        switch (nextState)
        {
            case STATE_TITLE:
            {
                if (oldSkin!=gameSettings->skin)
                {
                    oldSkin=gameSettings->skin;
                    glbEffect->loadImages();
                    loadImages();
                }

                currentState =new Title();
                break;
            }

            case STATE_OPTIONS:
            {
                currentState =new OptionsMenu();
                break;
            }

            case STATE_PROFILES:
            {
                currentState =new ProfileMenu();
                break;
            }

            case STATE_INGAME:
            {
                currentState =new InGame();
                break;
            }

            case STATE_ABOUT:
            {
                currentState = new AboutScreen();
                break;
            }

            case STATE_PROGRESS:
            {
                currentState = new ProgressInfo();
                break;
            }


        };
        stateID     =   nextState;
        nextState   =   STATE_NULL;
    }


}

void StateMachine::renderBackground(float phase)
{
    PROFILER_START();

    if (texBackground->usesColorModulation()!= gameSettings->translucentBackgrounds)
        texBackground->useColorMod(gameSettings->translucentBackgrounds);

    if (gameSettings->translucentBackgrounds )
    {
        float psine=0.75+0.25*mySine(phase/2);

        SDL_Color modColor={
                                    255 -(gameSettings->colors[COLOR_MAXVARIATION].r*psine)*(1.-mySine(phase)),
                                    255 -(gameSettings->colors[COLOR_MAXVARIATION].g*psine)*(1.-mySine(phase+PI*0.66)),
                                    255 -(gameSettings->colors[COLOR_MAXVARIATION].b*psine)*(1.-mySine(phase+PI*1.33))
                            };

        texBackground->setModColor(  modColor.r, modColor.g, modColor.b );
        gameSettings->modulateBGColor(modColor);
    }
    texBackground->render(0,0,screenRect);
/*
    SDL_SetRenderDrawColor(glbRenderer,
                           gameSettings->colors[COLOR_ORIGINAL_BACKGROUND].r,
                           gameSettings->colors[COLOR_ORIGINAL_BACKGROUND].g,
                           gameSettings->colors[COLOR_ORIGINAL_BACKGROUND].b,
                           255
                           );
    SDL_RenderClear(glbRenderer);
    */
    PROFILER_STOP();
}
//--------------------------------------------------

void StateMachine::Run()
{
    Timer gameTimer;//used for antihack check too
    Timer fps;
    float dt;
    int ticks=16;
    float phase=0.;

    float frameCap = refreshRate;

    framesPerSec = refreshRate;
    dt= 1./framesPerSec;

    int frameCount=0;

    gameTimer.Start();

#ifdef HACK_PROTECTION
    #ifdef PLATFORM_ANDROID
        int checkTime=100+genRand10000()%500;
    #endif
#endif // HACK_PROTECTION


#ifdef FPS_STATS
    int framesPassed=0;
    int processTime=0;
    int renderTime=0;
    int totalRenderTime=0;
    int totalTime=0;
#endif
    while( stateID != STATE_EXIT )
    {
#ifdef PROFILING
        glbProfiler->startRec("","Global::ActiveCycle");
        glbProfiler->startRec("","Global::PureCycle");
#endif
        fps.Start();

        currentState->HandleEvents();
        if (!isHibernating)
        {
            currentState->Logic( dt ,phase);
            glbEffect->update( dt ,phase);
        }


        ChangeState();

#ifdef FPS_STATS
        ticks=fps.GetTicks();
        processTime+=ticks;
#endif
        if (!isHibernating)
        {
            renderBackground(phase);

            currentState->Render(phase);
            ++frameCount;
#ifdef FPS_STATS
            ticks=fps.GetTicks()-ticks;
            renderTime+=ticks;
#endif

#ifdef PROFILING
            glbProfiler->stopRec("","Global::PureCycle");
#endif

            SDL_RenderPresent(glbRenderer);
#ifdef FPS_STATS
            ticks=fps.GetTicks()-ticks;
            totalRenderTime+=ticks;
#endif
        }
#ifdef PROFILING
        else
            glbProfiler->stopRec("","Global::PureCycle");

        glbProfiler->stopRec("","Global::ActiveCycle");
#endif

#ifdef CAP_FRAMERATE          //Cap the frame rate

        ticks=fps.GetTicks();
        if (isHibernating)
            frameCap=1;
        else
            frameCap = refreshRate;

        if( ticks < (1000. / frameCap) )
        {
            SDL_Delay( (int)( 1000. / frameCap ) - ticks );
        }
#endif
        if (!isHibernating)
        {
            ticks=fps.GetTicks();
#ifdef FPS_STATS
            totalTime+=ticks;
#endif // FPS_STATS
            framesPerSec = 1000./ticks;
            //framesPerSec=0.9*framesPerSec+(100./ticks);//using average
            dt= ticks/1000.;


            phase += PI*ticks/6000.; //PI every 6 seconds
            if (phase >210*PI) phase=0;

        }

#ifdef HACK_PROTECTION
    #ifndef DEMO_GAME
        #ifdef PLATFORM_ANDROID

        if (( !integrityCheckComplete)&&(gameTimer.GetTicks() >=checkTime) )
        {
            ahCheckIntegrity();
            ahCheckLicence();//to initiate the license check
            checkTime+=2000+genRand10000()%8000;//licence check will happen 2..10 secs after the integrity check
        }
        else
        if (( !licenceCheckComplete)&&(gameTimer.GetTicks() >=checkTime) )
        {
            ahCheckLicence();//trying to acquire the licence check result
            checkTime+=5000 + genRand10000()%5000;//all subsequent calls
        }
        #endif
    #endif
#endif // HACK_PROTECTION


#ifdef FPS_STATS
        framesPassed++;
        if (framesPassed==100)
        {

            sprintf(glbLogMsg,"PERFORMANCE FPS=[%.3f]====\n",framesPerSec);
            mySDL_Log(glbLogMsg);

            sprintf(glbLogMsg,"events-logic-updates [%.6f]\n",(1.*processTime)/totalTime);
            mySDL_Log(glbLogMsg);

            sprintf(glbLogMsg,"rendering            [%.6f]\n",(1.*renderTime)/totalTime);
            mySDL_Log(glbLogMsg);

            sprintf(glbLogMsg,"rendering total      [%.6f]\n",(1.*totalRenderTime)/totalTime);
            mySDL_Log(glbLogMsg);

            framesPassed=0;
            processTime=0;
            totalTime=0;
            renderTime=0;
            totalRenderTime=0;
        }
#endif
    }

    //END Stats
    sprintf(glbLogMsg,"frames: %d\n",frameCount);
    mySDL_Log(glbLogMsg);
    sprintf(glbLogMsg,"Total time: %d millis\n",gameTimer.GetTicks());
    mySDL_Log(glbLogMsg);
    framesPerSec=frameCount/(gameTimer.GetTicks()/1000.);
    sprintf(glbLogMsg,"Av. FPS:%.3f\n",framesPerSec);
    mySDL_Log(glbLogMsg);

    gameTimer.Stop();

}

//================ END OF STATEMACHINE IMPLEMENTATION ==============
