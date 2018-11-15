
#include "base.h"

#include "basefuncs.h"

#include "stateMachine.h"
#include "states.h"
#include "aboutScreen.h"

#include "texture.h"
#include "button.h"
#include "gameSettings.h"
//#include "levelStats.h"

#include "screenEffects.h"
#include "profiler.h"

#include <cstdio> //for sprintf()
#include <ctime>

#include "myMath.h"


AboutScreen::AboutScreen()
{
#ifdef CONSOLE_LOG
    sprintf(glbLogMsg,"AboutScreen\n");
    mySDL_Log(glbLogMsg);
#endif

    isHibernating=false;

    loadImages();

    loadPlayMusic("media/about.ogg");

  //  sndClick        = Mix_LoadWAV( "media/snd_menu.wav" );
    initButtons();

    scrollRect.x = guiBox->x+guiBox->w*0.05;
    scrollRect.y = guiBox->y+guiBox->h*0.17;
    scrollRect.w = guiBox->w*0.9;
    scrollRect.h = guiBox->h*0.7;

    initMovement();
    setAlphaValues();

    SDL_ShowCursor(SDL_ENABLE);

}

void AboutScreen::loadImages()
{
    handles[HANDLE_LOGO]    =   glbTexManager->insertTexture("logo.png",SKIN_UNIVERSAL,true);

    handles[HANDLE_BUTTONS] =   glbTexManager->insertTexture("buttons/buttons_purple.png",SKIN_UNIVERSAL,true);

    createMessages();
}


void AboutScreen::createMessages()
{
    char str[300];

    for (int i=0;i<NUM_MESSAGES;i++)
        texMessages[i]=new BaseTexture();

    int nextFree=0;

  //  SDL_Color colorText1={50,140,0};
  //  SDL_Color colorText2={180,60,0};
  //  SDL_Color colorText3={0,60,150};


    sprintf(str,"PuzzBob v%.3f%c",PROGRAM_VERSION,isLegitimateCopy?' ':'f');//in demo version legitimate copy is false
    texMessages[nextFree++]->fromText(str,gameSettings->colors[COLOR_NOBG2],glbFont);

{

    time_t tp;
    struct tm *now;

    time(&tp);
    now=localtime (&tp);

    sprintf(str,"by DetritusOnIce (c) 2015-%d",1900+now->tm_year);
}
    texMessages[nextFree++]->fromText(str,gameSettings->colors[COLOR_NOBG2],glbMessageFont);

    texMessages[nextFree++]->fromText("Design:",gameSettings->colors[COLOR_NOBG1],glbMessageFont);
    texMessages[nextFree++]->fromText("Programming:",gameSettings->colors[COLOR_NOBG1],glbMessageFont);
    texMessages[nextFree++]->fromText("Graphics:",gameSettings->colors[COLOR_NOBG1],glbMessageFont);
    texMessages[nextFree++]->fromText("Animation:",gameSettings->colors[COLOR_NOBG1],glbMessageFont);
    texMessages[nextFree++]->fromText("Sound Effects:",gameSettings->colors[COLOR_NOBG1],glbMessageFont);
    texMessages[nextFree++]->fromText("Thanasis Karpetis",gameSettings->colors[COLOR_NOBG2],glbFont);
    texMessages[nextFree++]->fromText("detritusonice@gmail.com",gameSettings->colors[COLOR_NOBG3],glbMessageFont);

    texMessages[nextFree++]->fromText("Testing:",gameSettings->colors[COLOR_NOBG1],glbMessageFont);
    texMessages[nextFree++]->fromText("Kat",gameSettings->colors[COLOR_NOBG2],glbMessageFont);
    texMessages[nextFree++]->fromText("Telmac",gameSettings->colors[COLOR_NOBG2],glbMessageFont);
    texMessages[nextFree++]->fromText("KoniKuno",gameSettings->colors[COLOR_NOBG2],glbMessageFont);
    texMessages[nextFree++]->fromText("Anhel",gameSettings->colors[COLOR_NOBG2],glbMessageFont);
    texMessages[nextFree++]->fromText("Zack",gameSettings->colors[COLOR_NOBG2],glbMessageFont);

    texMessages[nextFree++]->fromText("Music:",gameSettings->colors[COLOR_NOBG1],glbMessageFont);
    texMessages[nextFree++]->fromText("BenSound.com",gameSettings->colors[COLOR_NOBG2],glbMessageFont);
    texMessages[nextFree++]->fromText("http://www.bensound.com",gameSettings->colors[COLOR_NOBG3],glbMessageFont);

    texMessages[nextFree++]->fromText("Font:",gameSettings->colors[COLOR_NOBG1],glbMessageFont);
    texMessages[nextFree++]->fromText("Juergen Habich",gameSettings->colors[COLOR_NOBG2],glbMessageFont);
    texMessages[nextFree++]->fromText("juergen.tk",gameSettings->colors[COLOR_NOBG3],glbMessageFont);

    texMessages[nextFree++]->fromText("Made using:",gameSettings->colors[COLOR_NOBG1],glbMessageFont);
    texMessages[nextFree++]->fromText("CodeBlocks",gameSettings->colors[COLOR_NOBG2],glbMessageFont);
    texMessages[nextFree++]->fromText("MinGW",gameSettings->colors[COLOR_NOBG2],glbMessageFont);
    texMessages[nextFree++]->fromText("SDL 2",gameSettings->colors[COLOR_NOBG2],glbMessageFont);
    texMessages[nextFree++]->fromText("Android NDK",gameSettings->colors[COLOR_NOBG2],glbMessageFont);
    texMessages[nextFree++]->fromText("ANT",gameSettings->colors[COLOR_NOBG2],glbMessageFont);
    texMessages[nextFree++]->fromText("GIMP",gameSettings->colors[COLOR_NOBG2],glbMessageFont);
    texMessages[nextFree++]->fromText("Audacity",gameSettings->colors[COLOR_NOBG2],glbMessageFont);
    texMessages[nextFree++]->fromText("Tortoise SVN",gameSettings->colors[COLOR_NOBG2],glbMessageFont);

    texMessages[nextFree++]->fromText("Thanks go to:",gameSettings->colors[COLOR_NOBG1],glbMessageFont);
    texMessages[nextFree++]->fromText("LazyFoo",gameSettings->colors[COLOR_NOBG2],glbMessageFont);
    texMessages[nextFree++]->fromText("The creators of and contributors",gameSettings->colors[COLOR_NOBG2],glbMessageFont);
    texMessages[nextFree++]->fromText("to all the above tools.",gameSettings->colors[COLOR_NOBG2],glbMessageFont);
    texMessages[nextFree++]->fromText("Fellows at StackOverflow.",gameSettings->colors[COLOR_NOBG2],glbMessageFont);

    for (int i=0;i<nextFree;i++)
        texMessages[i]->useAlphaBlending(true);

}
//-----------------------------------------------

void AboutScreen::initButtons()
{

    closeButton= new SimpleButton( glbTexManager->getTexturePtr(handles[HANDLE_BUTTONS]),
                                           guiBox->x+guiBox->w/4,
                                           guiBox->y + guiBox->h*0.9,
                                           guiBox->w/2,
                                           guiUnit,
                                           "Close",
                                           BT_PUSHONLY,
                                           SIMPLE_BUTTON_ALPHA);

}

//-----------------------------------------------

AboutScreen::~AboutScreen()
{
    SDL_ShowCursor(SDL_DISABLE);

 //   Mix_HaltChannel(-1);

 //   Mix_FreeChunk(sndClick);

    delete closeButton;

    unloadImages();

    for (int i=0;i<NUM_MESSAGES;i++)
    {
        delete texMessages[i];
        texMessages[i]=NULL;
    }

}
void AboutScreen::unloadImages()
{
   //for (int i=0; i<NUM_HANDLES;i++)
   //  glbTexManager->removeTexture(handles[i]);
}
//-----------------------------------------------

void AboutScreen::HandleEvents()
{
    SDL_Event event;

    while (SDL_PollEvent(&event))
    {
        if ( (event.type==SDL_QUIT) || (event.type==SDL_APP_TERMINATING ))
            StateMachine::SetNextState(STATE_EXIT);
        else if ( (event.type==SDL_APP_WILLENTERBACKGROUND) ||
                 (event.type==SDL_APP_DIDENTERFOREGROUND ))
                 {
                     isHibernating=!isHibernating;

                     if (gameSettings->musicVolume>0)
                     {
                        if (isHibernating) Mix_PauseMusic();
                        else Mix_ResumeMusic();
                     }
                 }
        else if( event.type == SDL_WINDOWEVENT )
            { //Window resize/orientation change
                if( event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED )
                {
                    if ((event.window.data1!=screenRect->w)||
                        (event.window.data2!=screenRect->h) )
                    {
                        setScreenRect(event.window.data1,event.window.data2);

                        glbEffect->resize();

                        SDL_RenderPresent( glbRenderer );
                    }
                }
            }
        else if (event.type==SDL_KEYDOWN)
            {
                switch (event.key.keysym.sym)
                {
                    case SDLK_AC_BACK:
                    case SDLK_ESCAPE: StateMachine::SetNextState(STATE_TITLE);break;

                    default:break;
                };

            }
        else if (event.type==SDL_MOUSEMOTION)
        {
            glbEffect->moveEffect(FPoint(event.motion.x, event.motion.y));
        }
        /*
        else if (event.type==SDL_MOUSEWHEEL)
        {
            if (event.wheel.y==1)
                glbEffect->changeEffectAlpha(5);
        }
        else if (event.type==SDL_MOUSEBUTTONDOWN)
        {
            //glbEffect->moveEffect(FPoint(event.motion.x, event.motion.y));
        }
        */
    }

     closeButton->handleEvents();

     if (closeButton->getState()==BUTTONSTATE_PUSHEDHOVER)
        {
            //Mix_PlayChannel( -1, sndClick, 0 );

            StateMachine::SetNextState(STATE_TITLE);
        }
}


//-----------------------------------------------

void AboutScreen::Logic(float dt,float phase)
{
    moveObjects(dt);
    setAlphaValues();
}

//-----------------------------------------------

void AboutScreen::Render(float phase)
{
    PROFILER_START();

    glbEffect->render();

    BaseTexture* texLogo = glbTexManager->getTexturePtr(handles[HANDLE_LOGO]);

    SDL_Rect rc={0,0,scrollRect.w,(int)(scrollRect.w*1.*texLogo->getHeight()/texLogo->getWidth())};

    texLogo->render( scrollRect.x, guiBox->y+guiBox->h*0.02,&rc);

    for (int i=0;i<NUM_MESSAGES;i++)
    {
        if (alpha[i]>0)
            texMessages[i]->render(positions[i].x,leadpoint+positions[i].y,NULL,-1,alpha[i]);
    }

    closeButton->render();

    PROFILER_STOP();
}

//-----------------------------------------------------------------------------------
void AboutScreen::initMovement()
{
    velocity=-guiBox->h/12.;

    setGaps();

    for (int i=0;i<NUM_MESSAGES;i++)
    {
        dimensions[i].x = texMessages[i]->getWidth();
        dimensions[i].y = texMessages[i]->getHeight();
    }

    leadpoint=scrollRect.y+scrollRect.h;

    positions[0].y=0;
    positions[0].x= scrollRect.x+(scrollRect.w-dimensions[0].x)/2;

    for(int i=1;i<NUM_MESSAGES;i++)
    {
        positions[i].y=positions[i-1].y+dimensions[i-1].y + gaps[i];
        positions[i].x= scrollRect.x+(scrollRect.w-dimensions[i].x)/2;
    }

}
//-----------------------------------------------------------------------------------
void AboutScreen::moveObjects(float dt)
{
    leadpoint+=velocity*dt;

    if (leadpoint+positions[NUM_MESSAGES-1].y+dimensions[NUM_MESSAGES-1].y <scrollRect.y)
        leadpoint=scrollRect.y+scrollRect.h;

}

void AboutScreen::setAlphaValues()
{
    for (int i=0;i<NUM_MESSAGES;i++)
    {
        int start=std::max(scrollRect.y,int(leadpoint+positions[i].y));
        int stop=std::min(scrollRect.y+scrollRect.h,int(leadpoint + positions[i].y+dimensions[i].y));

        if ( (stop-start>0) &&(dimensions[i].y>0))
            alpha[i]=(stop-start)*255./dimensions[i].y;
        else alpha[i]=0;
    }
}

void AboutScreen::setGaps()
{
    int index=0;

    gaps[index++]=guiUnit*0.f;
    gaps[index++]=guiUnit*0.6f;

    gaps[index++]=guiUnit*3.f;
    gaps[index++]=guiUnit*0.5f;
    gaps[index++]=guiUnit*0.5f;
    gaps[index++]=guiUnit*0.5f;
    gaps[index++]=guiUnit*0.5f;
    gaps[index++]=guiUnit*1.f;
    gaps[index++]=guiUnit*.3f;

    gaps[index++]=guiUnit*4.f;
    gaps[index++]=guiUnit*0.5f;
    gaps[index++]=guiUnit*0.3f;
    gaps[index++]=guiUnit*0.5f;
    gaps[index++]=guiUnit*0.3f;
    gaps[index++]=guiUnit*0.3f;

    gaps[index++]=guiUnit*3.f;
    gaps[index++]=guiUnit*0.5f;
    gaps[index++]=guiUnit*0.3f;

    gaps[index++]=guiUnit*2.f;
    gaps[index++]=guiUnit*0.5f;
    gaps[index++]=guiUnit*0.3f;

    gaps[index++]=guiUnit*2.f;
    gaps[index++]=guiUnit*0.5f;
    gaps[index++]=guiUnit*0.3f;
    gaps[index++]=guiUnit*0.3f;
    gaps[index++]=guiUnit*0.3f;
    gaps[index++]=guiUnit*0.3f;
    gaps[index++]=guiUnit*0.3f;
    gaps[index++]=guiUnit*0.3f;
    gaps[index++]=guiUnit*0.3f;

    gaps[index++]=guiUnit*2.f;
    gaps[index++]=guiUnit*0.8f;
    gaps[index++]=guiUnit*0.5f;
    gaps[index++]=guiUnit*0.2f;
    gaps[index++]=guiUnit*0.5f;

}
