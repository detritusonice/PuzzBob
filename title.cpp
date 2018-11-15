#include "base.h"

#include "basefuncs.h"

#include "stateMachine.h"
#include "states.h"
#include "title.h"

#include "texture.h"
#include "button.h"
#include "gameSettings.h"
#include "levelStats.h"

#include "screenEffects.h"
//#include "localEffects.h"

#include <cstdio> //for sprintf()
//#include <cmath> //for sin()
#include "myMath.h"


Title::Title()
{
#ifdef CONSOLE_LOG
    sprintf(glbLogMsg,"Title\n");
    mySDL_Log(glbLogMsg);
#endif

    isHibernating=false;

    loadImages();

    loadPlayMusic("media/title.ogg");

  //  sndClick        = Mix_LoadWAV( "media/snd_menu.wav" );

    initButtons();

    SDL_ShowCursor(SDL_ENABLE);

}

void Title::loadImages()
{
    handles[HANDLE_LOGO]    =   glbTexManager->insertTexture("logo.png",SKIN_UNIVERSAL,true);

    handles[HANDLE_BUTTONS] =   glbTexManager->insertTexture("buttons/buttons_purple.png",SKIN_UNIVERSAL,true);

    handles[HANDLE_PROFILE_BACKGROUND] = glbTexManager->insertTexture("buttons/buttons_glow.png",SKIN_UNIVERSAL,true);

    texVersion = new BaseTexture();

    char version[40];
    sprintf(version,"PuzzBob v%.3f%c",PROGRAM_VERSION,isLegitimateCopy?' ':'f');//in demo version legitimate copy is false

    texVersion->fromText( version);

    texWelcome = new BaseTexture();
    texWelcome->fromText( "Welcome",gameSettings->colors[COLOR_NOBG4]);

    texProfileHelp = new BaseTexture();
    texProfileHelp->fromText( "tap to change player",gameSettings->colors[COLOR_NOBG4]);

}
//-----------------------------------------------

void Title::initButtons()
{
    const char* strTitles[NUM_TITLE_BUTTONS]=
    { "Play","Options","About","Exit","progress",""};

    const float buttonY[NUM_TITLE_BUTTONS]=
   // {4.2,5.4,7.2,8.4,2.5,1};
    //{3,4.2,6,7.2,1};// 7.8,9,1};
    {3,4.2,7.,8.2,5.5,1};
    //{1,2.2,4,5.2,8};

    int buttonWidth=guiBox->w/2;
    int offsetX=(guiBox->w-buttonWidth)/2;

    for (int i=0;i<NUM_TITLE_BUTTONS-2;i++)
        titleButtons[i]= new SimpleButton( glbTexManager->getTexturePtr(handles[HANDLE_BUTTONS]),
                                           guiBox->x+offsetX,
                                           guiBox->y + buttonY[i]*guiUnit,
                                           buttonWidth,
                                           guiUnit,
                                           strTitles[i],
                                           BT_PUSHONLY,
                                           SIMPLE_BUTTON_ALPHA);

        titleButtons[ID_BTN_PROGRESS]= new SimpleButton( glbTexManager->getTexturePtr(handles[HANDLE_PROFILE_BACKGROUND]),
                                           guiBox->x+offsetX,
                                           guiBox->y + buttonY[ID_BTN_PROGRESS]*guiUnit,
                                           buttonWidth,
                                           guiUnit,
                                           strTitles[ID_BTN_PROGRESS],
                                           BT_PUSHONLY,
                                           SIMPLE_BUTTON_ALPHA);


        titleButtons[ID_BTN_PROFILE]= new SimpleButton( glbTexManager->getTexturePtr(handles[HANDLE_PROFILE_BACKGROUND]),
                                           guiBox->x+offsetX,
                                           guiBox->y + buttonY[ID_BTN_PROFILE]*guiUnit,
                                           buttonWidth,
                                           guiUnit,
                                           playerStats->getPlayerName().c_str(),
                                           BT_PUSHONLY,
                                           SIMPLE_BUTTON_ALPHA);
        rcWelcome.w=buttonWidth;
        rcWelcome.h=guiUnit;
        rcWelcome.x=guiBox->x+offsetX+ (rcWelcome.w-texWelcome->getWidth())/2;
        rcWelcome.y=guiBox->y + (buttonY[ID_BTN_PROFILE])*guiUnit-texWelcome->getHeight();

        rcProfileHelp.w=texProfileHelp->getWidth();
        rcProfileHelp.h=rcProfileHelp.w*texProfileHelp->getHeight()/texProfileHelp->getWidth();

        rcProfileHelp.x=guiBox->x+(guiBox->w- rcProfileHelp.w)/2;
        rcProfileHelp.y=guiBox->y + (buttonY[ID_BTN_PROFILE]+0.9)*guiUnit;

}

//-----------------------------------------------

Title::~Title()
{
    SDL_ShowCursor(SDL_DISABLE);

 //   Mix_HaltChannel(-1);

 //   Mix_FreeChunk(sndClick);

    for (int i=0;i<NUM_TITLE_BUTTONS;++i)
    {
#ifdef CONSOLE_LOG
        sprintf(glbLogMsg,"deleting button %d\n",i);
        mySDL_Log(glbLogMsg);
#endif
        delete titleButtons[i];
    }

    unloadImages();

    delete texVersion;
    delete texWelcome;
    delete texProfileHelp;

//    delete texDebugInfo;

}
void Title::unloadImages()
{
   //for (int i=0; i<NUM_HANDLES;i++)
   //  glbTexManager->removeTexture(handles[i]);
}
//-----------------------------------------------

void Title::HandleEvents()
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
                         if (isHibernating) Mix_PauseMusic();
                           else Mix_ResumeMusic();
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
                    case SDLK_ESCAPE: StateMachine::SetNextState(STATE_EXIT);break;

                    case SDLK_o: StateMachine::SetNextState(STATE_OPTIONS);break;

                    case SDLK_p: StateMachine::SetNextState(STATE_PROFILES);break;

                    case SDLK_i: StateMachine::SetNextState(STATE_PROGRESS);break;

                    case SDLK_RETURN: StateMachine::SetNextState(STATE_INGAME);break;

                    case SDLK_a: StateMachine::SetNextState(STATE_ABOUT);break;

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

     for (int i=0;i<NUM_TITLE_BUTTONS;i++)
        titleButtons[i]->handleEvents();

    for ( int i=0;i<NUM_TITLE_BUTTONS;i++)
        if (titleButtons[i]->getState()==BUTTONSTATE_PUSHEDHOVER)
        {
            //Mix_PlayChannel( -1, sndClick, 0 );

            switch(i)
            {
                case ID_BTN_PLAY: { StateMachine::SetNextState(STATE_INGAME); break;}
                case ID_BTN_MENU: {StateMachine::SetNextState(STATE_OPTIONS); break;}
                case ID_BTN_PROFILE: {StateMachine::SetNextState(STATE_PROFILES); break;}
                case ID_BTN_ABOUT: {StateMachine::SetNextState(STATE_ABOUT); break;}
                case ID_BTN_EXIT: {StateMachine::SetNextState(STATE_EXIT); break;}
                case ID_BTN_PROGRESS: {StateMachine::SetNextState(STATE_PROGRESS); break;}
                default:break;
            };

           break;
        }
}


//-----------------------------------------------

void Title::Logic(float dt,float phase)
{

}

//-----------------------------------------------

void Title::Render(float phase)
{
    glbEffect->render();

    BaseTexture* texLogo = glbTexManager->getTexturePtr(handles[HANDLE_LOGO]);

    SDL_Rect rc={0,0,(int)(0.9*guiBox->w),(int)(0.9*guiBox->w*texLogo->getHeight()/texLogo->getWidth())};

    texLogo->render( guiBox->x+(guiBox->w-rc.w)/2., guiBox->y+0.7*guiBox->h,&rc);

//    texDebugInfo->render(guiBox->x+(guiBox->w - texDebugInfo->getWidth())/2,
//                       guiBox->y + guiBox->h - (int)guiUnit - texDebugInfo->getHeight());

    texVersion->render(guiBox->x+(guiBox->w - texVersion->getWidth())/2,
                       guiBox->y + guiBox->h - texVersion->getHeight());

    float a=myfabs(mySine(phase));
    a*=a;
    a*=a;
    a*=a;

    texWelcome->render(rcWelcome.x,rcWelcome.y,NULL);
    texProfileHelp->render(rcProfileHelp.x, rcProfileHelp.y,&rcProfileHelp,-1,150*a);

    for (int i=0;i<NUM_TITLE_BUTTONS;i++)
        titleButtons[i]->render();


//    SDL_RenderPresent(glbRenderer);//this is called by the state machine to facilitate profiling


}



