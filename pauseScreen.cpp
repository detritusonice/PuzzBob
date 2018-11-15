
#include "base.h"
#include "pauseScreen.h"

#include "texture.h"
#include "button.h"
#include "levelStats.h"
#include "basefuncs.h"
#include "gameSettings.h"
#include "inGame.h"

#include <cstdio>

//===================================================================================================

PauseScreen::PauseScreen(const SDL_Rect& rcWell, int playingLevel, bool isQuit ,bool gameInProgress, int roundTime, int roundShots, int roundScore, bool makeAggregates)
{
    rc=rcWell;
    done = false;

    level=playingLevel;

    isPauseScreen=!isQuit;
    isGameInProgress = gameInProgress;

    quitGame=false;

    duration=roundTime;
    shots=roundShots;
    score = roundScore;

    loadImages();

    initButtons();

    createMessages();
    if (makeAggregates)
        createStaticMessageImage();

}

//------------------------------------------------------------------------------------------------------

PauseScreen::~PauseScreen()
{
    int i;
    for (i=0;i<NUM_PAUSE_BUTTONS;i++)
        delete buttons[i];

    for (i=0;i<NUM_MESSAGES;i++)
        if (msg[i]!=NULL)
            delete msg[i];

    if (texTotalMessages!=NULL)
        delete texTotalMessages;

    texButtons[0]=texButtons[1]=NULL;

    texBackground=NULL;
}

//------------------------------------------------------------------------------------------------------

void PauseScreen::initButtons()
{
    const char* captions[]={
                            (isPauseScreen)?
                            "Continue Game"
                            :"Yes, Quit",
                            (isPauseScreen)?
                            "Quit Game"
                            :"Contine Game",
};

    for (int i=0;i<NUM_PAUSE_BUTTONS;i++)
    {
        buttons[i]= new SimpleButton(texButtons[i],rc.x + rc.w/4,
                                     rc.y + rc.h*0.7 +1.2*i*guiUnit ,
                                     rc.w/2,guiUnit,
                                     captions[i],
                                     BT_PUSHONLY,SIMPLE_BUTTON_ALPHA  );
    }

}

//------------------------------------------------------------------------------------------------------

void PauseScreen::loadImages()
{
    texBackground = glbTexManager->getTexturePtr(glbTexManager->insertTexture("message_background.png",SKIN_UNIVERSAL,true));

    int index_go,index_quit;


    if ( isPauseScreen)
    {
        index_go=0;
        index_quit=1;
    }
    else
    {
        index_go=1;
        index_quit=0;
    }
    texButtons[index_go]    = glbTexManager->getTexturePtr(glbTexManager->insertTexture("buttons/buttons_green.png",SKIN_UNIVERSAL,true));
    texButtons[index_quit]    = glbTexManager->getTexturePtr(glbTexManager->insertTexture("buttons/buttons_cyan.png",SKIN_UNIVERSAL,true));

    for (int i=0;i<NUM_MESSAGES;i++)
        msg[i]= new BaseTexture;

    texTotalMessages=NULL;

}

//------------------------------------------------------------------------------------------------------

void PauseScreen::createMessages()
{
    for (int i=0;i<NUM_MESSAGES;i++)
        msg[i]->fromText(" ");

    bool passed=playerStats->hasBeaten(level);

//    SDL_Color colorText1={0,60,180};
 //   SDL_Color colorText2={255,120,0};
 //   SDL_Color colorText3={30,160,0};


    if (isPauseScreen)
        msg[0]->fromText("Game is Paused",glbTextColor,glbFont);
    else
        msg[0]->fromText("Quit your Game?",glbTextColor,glbFont);

    char str[80];

    if (isGameInProgress)
        sprintf(str,"%s playing level: %d",playerStats->getPlayerName().c_str(),level+1);
    else
        sprintf(str,"%s starting level: %d",playerStats->getPlayerName().c_str(),level+1);
    msg[1]->fromText(str,gameSettings->colors[COLOR_TEXT4],glbMessageFont, rc.w-guiUnit);

    const char* prStr[NUM_RANDOMNESS_LEVELS]={"Low","Moderate","High","Highest"};

    sprintf(str,"Match:             %d\nRandomness:    %s\nPressure:        %s",
            gameSettings->matchNumber,prStr[gameSettings->randomness],prStr[gameSettings->pressure]);
    msg[2]->fromText(str,gameSettings->colors[COLOR_TEXT2],glbMessageFont, rc.w-guiUnit);


    int nextFree=3;

    if (gameSettings->showStatsOnPause)
    {
        sprintf(str,"Round score:     %d",score);
        msg[nextFree++]->fromText(str,gameSettings->colors[COLOR_TEXT1]);

        if (passed)
            sprintf(str,"Best  score:      %d",playerStats->getRoundScore(level));
        else sprintf(str,"Best  score:      --");
        msg[nextFree++]->fromText(str,gameSettings->colors[COLOR_TEXT3]);

        int mintime=duration;//playerStats->getMinTime(lastLevel);
        sprintf(str,"Round Time:      %d.%03d s",mintime/1000,mintime%1000);
        msg[nextFree++]->fromText(str,gameSettings->colors[COLOR_TEXT1]);

        if (passed)
        {
            mintime=playerStats->getMinTime(level);
            sprintf(str,"Best  Time:       %d.%03d s",mintime/1000,mintime%1000);
        }
        else sprintf(str,"Best  Time:      ----");
        msg[nextFree++]->fromText(str,gameSettings->colors[COLOR_TEXT3]);

        sprintf(str,"Round Shots:     %d",shots);
        msg[nextFree++]->fromText(str,gameSettings->colors[COLOR_TEXT1]);

        if (passed)
            sprintf(str,"Least Shots:      %d",playerStats->getMinShots(level));
        else
            sprintf(str,"Least Shots:     --");
        msg[nextFree++]->fromText(str,gameSettings->colors[COLOR_TEXT3]);

    }

    if (isPauseScreen)
        msg[nextFree++]->fromText("Press Back Button to Continue.\n",glbTextColor,glbFont, rc.w-guiUnit);
    else
    {
        msg[nextFree++]->fromText("Press Back Button to Quit.\n",glbTextColor,glbFont, rc.w-guiUnit);
        if (isGameInProgress)
            msg[nextFree]->fromText("Your game will be saved.",gameSettings->colors[COLOR_TEXT2],glbMessageFont, rc.w-guiUnit);
        else
        {
            msg[nextFree++]->fromText("Your progress is already saved.",glbTextColor,glbMessageFont, rc.w-guiUnit);
            msg[nextFree++]->fromText("Saved game will not be altered.",gameSettings->colors[COLOR_TEXT2],glbMessageFont, rc.w-guiUnit);
        }
    }

}
//------------------------------------------------------------------------------------------------------

bool PauseScreen::isDone ( bool *playOn )
{
    if (done)
    {
        *playOn=!quitGame;
    }

    return done;
}
//------------------------------------------------------------------------------------------------------

void PauseScreen::handleEvents(SDL_Event& event)
{
    int i;

    for (i=0;i<NUM_PAUSE_BUTTONS;i++)
        buttons[i]->handleEvents();

    for (i=0;i<NUM_PAUSE_BUTTONS;i++)
       if (buttons[i]->getState()==BUTTONSTATE_PUSHEDHOVER)
        {
            switch (i)
            {
                case ID_BTN_1:  quitGame=!isPauseScreen;break;

                case ID_BTN_2:  quitGame=isPauseScreen; break;
            }
            done=true;

            return;
        }
}
//------------------------------------------------------------------------------------------------------
void PauseScreen::createStaticMessageImage()
{
    texTotalMessages = new BaseTexture;

    if (!texTotalMessages->createBlank(rc.w,rc.h,SDL_TEXTUREACCESS_TARGET) )
    {
#ifdef CONSOLE_LOG
        sprintf(glbLogMsg, "InGame:texTotalMessages creation Failed\n");
        mySDL_Log(glbLogMsg);
#endif // CONSOLE_LOG

        delete texTotalMessages;
        texTotalMessages=NULL;
    }
    else
        texTotalMessages->useAlphaBlending(true);

    if (texTotalMessages!=NULL)
    {
        texTotalMessages->setAsRenderTarget();

        FPoint offset( -rc.x, -rc.y );

        renderMessages(offset);

        texTotalMessages->resetRenderTarget();
    }

}
//------------------------------------------------------------------------------------------------------
void PauseScreen::renderMessages(const FPoint& offset)
{
    float dy[NUM_MESSAGES]={guiUnit*1.1f,guiUnit*0.8f,guiUnit*1.7f,guiUnit*0.5f,
                            guiUnit*0.8f,guiUnit*0.5f,guiUnit*0.8f,guiUnit*0.5f,
                            guiUnit*0.8f,guiUnit*1.0f,guiUnit*0.5f,guiUnit*0.9f};

    int px,py,i;

    px = offset.x + rc.x + guiUnit/2;
    py = offset.y + rc.y + guiUnit/2;

    for (i=0;i<NUM_MESSAGES;i++)
    {
        msg[i]->render(px,py);
        py+=dy[i];
    }

}
//------------------------------------------------------------------------------------------------------

void PauseScreen::render(float phase)
{
    texBackground->render( rc.x , rc.y,&rc,-1,180);

    if (texTotalMessages!=NULL)
        texTotalMessages->render(rc.x,rc.y);
    else
        renderMessages();


    for (int i=0;i<NUM_PAUSE_BUTTONS;i++)
        buttons[i]->render();
}



