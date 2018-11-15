#include "base.h"

#include "basefuncs.h"

#include "stateMachine.h"
#include "states.h"
#include "optionsMenu.h"

#include "texture.h"
#include "button.h"

#include "screenEffects.h"
#include "gameSettings.h"

#include <cstdio> //for sprintf()

#include <cmath>
#include "myMath.h"

#include "sliderBar.h"
#include "levelStats.h"


OptionsMenu::OptionsMenu()
{
#ifdef CONSOLE_LOG
    sprintf(glbLogMsg,"OptionsMenu\n");
    mySDL_Log(glbLogMsg);
#endif
    isHibernating=false;

    changedSkin=false;

    texButtons = glbTexManager->getTexturePtr(
                        glbTexManager->insertTexture("buttons/buttons_cyan.png",SKIN_UNIVERSAL,true));

    randomness     = gameSettings->randomness;
    pressure       = gameSettings->pressure;
    skin           = gameSettings->skin;
    match          = gameSettings->matchNumber;
    useAid         = gameSettings->useBallIndex;
    useBomb        = gameSettings->useBombs;
    useChrome      = gameSettings->useChromeBalls;
    useShifters      = gameSettings->useShifters;
    usePainters      = gameSettings->usePainters;
    useLevelers      = gameSettings->useLevelers;

    widthID = gameSettings->wellWidthID;

    renderBgEffect = gameSettings->renderBackgroundEffect;
    useTranslucency= gameSettings->translucentBackgrounds;
    multiExplode  = gameSettings->multiBallExplosions;
    useShine       = gameSettings->useShineEffect;
    fullSceneComplexity       = gameSettings->fullSceneComplexity;
    renderTrails   = gameSettings->renderTrails;
    flashOnExplosions = gameSettings->flashOnExplosions;

    showRoundScore = gameSettings->showRoundScore;
    showRoundTime  = gameSettings->showRoundTime;
    showRoundShots = gameSettings->showRoundShots;
    showBest       = gameSettings->showBest;
    showStatsOnPause = gameSettings->showStatsOnPause;
    showBeatenStats  = gameSettings->showBeatenStats;
    showSwipeHint    = gameSettings->showSwipeHint;
    overrideSettingsOnContinue = gameSettings->overrideSettingsOnContinue;
    musicVolume      = gameSettings->musicVolume;
    effectsVolume    = gameSettings->sfxVolume;

    initButtons();

    SDL_Rect handlerRect={0,0,(int)( guiBox->w-0.1*guiUnit),(int)(guiBox->h-1.7*guiUnit)};

    pageHandler = new AxialPageHandler(
                                       HORIZONTAL,true,
                                       FPoint( guiBox->x+ (guiBox->w-handlerRect.w)/2,
                                              guiBox->y+guiUnit/20),
                                       handlerRect,70);

    populatePages();

  //  sndClick        = Mix_LoadWAV( "media/snd_menu.wav" );

    SDL_ShowCursor(SDL_ENABLE);


}
//-----------------------------------------------
void OptionsMenu::initButtons()
{

    const char* strOptionsMenus[NUM_MENU_BUTTONS]=
    { "Cancel","OK"};

    float buttonWidth=0.4*guiBox->w;
    float columnOffset=buttonWidth+guiUnit;

    for (int i=0;i<NUM_MENU_BUTTONS;i++)
        menuButtons[i]= new SimpleButton(texButtons,
                                          screenRect->w/2 +(i-1)*columnOffset +0.5*guiUnit,
                                          guiBox->y + guiBox->h- 1.5*guiUnit,
                                          buttonWidth,
                                          guiUnit,
                                          strOptionsMenus[i],
                                          BT_PUSHONLY,
                                          SIMPLE_BUTTON_ALPHA);

}
//-----------------------------------------------
OptionsMenu::~OptionsMenu()
{
    SDL_ShowCursor(SDL_DISABLE);

 //   Mix_HaltChannel(-1);

 //   Mix_FreeChunk(sndClick);

    delete pageHandler;


    for (int i=0;i<NUM_MENU_BUTTONS;++i)
    {
#ifdef CONSOLE_LOG
        sprintf(glbLogMsg,"deleting button %d\n",i);
        mySDL_Log(glbLogMsg);
#endif
        delete menuButtons[i];
        menuButtons[i]=NULL;
    }

    texButtons=NULL;//texture manager deletes them

    if (changedSkin)
        gameSettings->setSkin(skin);

}

//-----------------------------------------------

void OptionsMenu::HandleEvents()
{
    SDL_Event event;

    while (SDL_PollEvent(&event))
    {
        if ( (event.type==SDL_QUIT) || (event.type==SDL_APP_TERMINATING ))
            StateMachine::SetNextState(STATE_EXIT);
        else
            if ( (event.type==SDL_APP_WILLENTERBACKGROUND) ||
                 (event.type==SDL_APP_DIDENTERFOREGROUND ))
                 {
                     isHibernating=!isHibernating;
                     if (musicVolume>0)// a pointer is passed, so it reflects changes made in the respecting page
                     {
                         if (isHibernating) Mix_PauseMusic();
                            else Mix_ResumeMusic();
                     }
                 }
        else if( event.type == SDL_WINDOWEVENT )
            { //Window resize/orientation change
                if( event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED )
                {
                    setScreenRect(event.window.data1,event.window.data2);

                    glbEffect->resize();

                    SDL_RenderPresent( glbRenderer );
                }
            }
        else if (event.type==SDL_KEYDOWN)
            {
                switch (event.key.keysym.sym)
                {
                    case SDLK_AC_BACK:
                    case SDLK_ESCAPE: StateMachine::SetNextState(STATE_TITLE);break;
                    //case SDLK_d: pageHandler->removeCurrentPage();break;

                    case SDLK_RETURN:   saveSettings();

                                        StateMachine::SetNextState(STATE_TITLE);
                                        break;
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
                if (event.wheel.y==-1)
                    glbEffect->changeEffectAlpha(-5);
            }
            */
    }
    bool handled =false;

    for (int i=0;i<NUM_MENU_BUTTONS;i++)
        handled |= menuButtons[i]->handleEvents();

    if (!handled)
        pageHandler->handleEvents();

    for ( int i=0;i<NUM_MENU_BUTTONS;i++)
        if (menuButtons[i]->getState()==BUTTONSTATE_PUSHEDHOVER)
        {
            //Mix_PlayChannel( -1, sndClick, 0 );
            switch(i)
            {
                case ID_BTN_CANCEL:
                        StateMachine::SetNextState(STATE_TITLE);break;

                case ID_BTN_OK:
                        saveSettings();
                        StateMachine::SetNextState(STATE_TITLE);
                        break;

                default:break;
            };

           break;
        }

}

//----------------------------------------------

void OptionsMenu::saveSettings()
{
    gameSettings->randomness  = randomness;
    gameSettings->pressure    = pressure;
    gameSettings->matchNumber = match;
    gameSettings->useBallIndex= useAid;
    gameSettings->useBombs = useBomb;
    gameSettings->useChromeBalls = useChrome;
    gameSettings->useShifters = useShifters;
    gameSettings->usePainters = usePainters;
    gameSettings->useLevelers = useLevelers;
    gameSettings->wellWidthID = widthID;

    gameSettings->renderBackgroundEffect=renderBgEffect;
    gameSettings->translucentBackgrounds=useTranslucency;
    gameSettings->multiBallExplosions=multiExplode;
    gameSettings->useShineEffect=useShine;
    gameSettings->renderTrails = renderTrails;
    gameSettings->flashOnExplosions = flashOnExplosions;

    gameSettings->musicVolume= musicVolume;
    gameSettings->sfxVolume= effectsVolume;

    gameSettings->showRoundScore = showRoundScore;
    gameSettings->showRoundTime = showRoundTime;
    gameSettings->showRoundShots = showRoundShots;
    gameSettings->showBest = showBest;
    gameSettings->showStatsOnPause = showStatsOnPause;
    gameSettings->showBeatenStats = showBeatenStats;
    gameSettings->showSwipeHint = showSwipeHint;
    gameSettings->overrideSettingsOnContinue = overrideSettingsOnContinue;


    if (gameSettings->fullSceneComplexity != fullSceneComplexity)
    {
        gameSettings->fullSceneComplexity = fullSceneComplexity;
        glbEffect->resize();//causes effect rectangles to be reco
    }

    if (skin!=gameSettings->skin)
    {
        gameSettings->skin=skin;
        changedSkin=true;
    }
    if (!gameSettings->translucentBackgrounds)
        gameSettings->resetBGColor();

    gameSettings->writeSettings();
}

//-----------------------------------------------

void OptionsMenu::Logic(float dt, float phase)
{
    pageHandler->update(dt,phase);
}

//-----------------------------------------------

void OptionsMenu::Render(float phase)
{

    glbEffect->render();

    pageHandler->render(phase);

    for (int i=0;i<NUM_MENU_BUTTONS;i++)
        menuButtons[i]->render();

    //    SDL_RenderPresent(glbRenderer);//this is called by the state machine to facilitate profiling
}
//-----------------------------------------------
void OptionsMenu::populatePages()
{

    SDL_Rect rc = { 0, 0, (int)(guiBox->w - 0.25*guiUnit),
                         (int)(guiBox->h - 1.85*guiUnit)};

    FPoint pos(guiBox->x+(guiBox->w-rc.w)/2.,guiBox->y +guiUnit/8);//guiUnit/8);

    pageHandler->addPage( new GamePlayPage("GamePlay",pos,rc,&randomness,&pressure,&match, &widthID,&useAid,&useBomb,&useChrome,&useLevelers,&useShifters,&usePainters) );

    pageHandler->addPage( new SkinPage("Themes",pos,rc,&skin,false) );

    bool addThemes2=false;
    for (int i=UNL_SKIN_DARK2;i<=UNL_SKIN_GEOMETRIC2;i++)
        if (playerStats->getUnlockableStatus(i))
        {
            addThemes2=true;
            break;
        }

    if (addThemes2)
        pageHandler->addPage( new SkinPage("Unlockable Themes",pos,rc,&skin,true) );

    pageHandler->addPage( new EffectsPage("Effects",pos,rc,&renderBgEffect, &useTranslucency, &multiExplode,&useShine,&fullSceneComplexity,&renderTrails, &flashOnExplosions,&musicVolume,&effectsVolume));

    pageHandler->addPage( new InfoPage("GameInfo",pos,rc,&showRoundScore, &showRoundTime, &showRoundShots,&showBest, &showStatsOnPause, &showBeatenStats, &showSwipeHint,&overrideSettingsOnContinue));

    pageHandler->doneInserting();

}



//=============================================================================


GamePlayPage::GamePlayPage(std::string titleStr, const FPoint& pos, const SDL_Rect& rect, int* rand, int* pres, int* match, int *widthID, bool* aid,
                                                bool *bomb, bool *chrome, bool *leveler, bool *shifter, bool *painter  )
{
    pressure     = pres;
    randomness   = rand;
    matchNumber  = match;
    wellWidthID  = widthID;
    useAimingAid = aid;
    useBombs = bomb;
    useChrome = chrome;
    useLeveler = leveler;
    usePainter = painter;
    useShifter = shifter;

    initPage(titleStr,pos,rect,255);

    loadImages();

    initControls();
}
//-----------------------------------------------------------------------------------
void GamePlayPage::loadImages()
{

    imageHandle = glbTexManager->insertTexture("page_background.png",SKIN_UNIVERSAL,true);

    texMsgBackground = glbTexManager->getTexturePtr(
                        glbTexManager->insertTexture("message_background.png",SKIN_UNIVERSAL,true));

    texMsgBackground->useAlphaBlending(true);


    texButtons[0] = glbTexManager->getTexturePtr(
                        glbTexManager->insertTexture("buttons/buttons_purple.png",SKIN_UNIVERSAL,true));

    texButtons[1] = glbTexManager->getTexturePtr(
                        glbTexManager->insertTexture("buttons/buttons_orange.png",SKIN_UNIVERSAL,true));

    texButtons[2] = glbTexManager->getTexturePtr(
                        glbTexManager->insertTexture("buttons/buttons_green.png",SKIN_UNIVERSAL,true));

    texButtons[3] = glbTexManager->getTexturePtr(
                        glbTexManager->insertTexture("buttons/buttons_yellow.png",SKIN_UNIVERSAL,true));


    texMessageRand = new BaseTexture();
    texMessagePres = new BaseTexture();

    texMessageAim  = new BaseTexture();
    texMessageMatch = new BaseTexture();
    texMatchNumber = new BaseTexture();
    texMessageBombs =new BaseTexture();
    texMessageChrome = new BaseTexture();
    texMessageShifter = new BaseTexture();
    texMessagePainter = new BaseTexture();
    texMessageLeveler = new BaseTexture();
    texMessageWidth = new BaseTexture();

    texMessageAim->fromText("Use Aiming Aid");
    texMessageBombs->fromText("Use Bombs");
    texMessageChrome->fromText("Use Chrome Balls");
    texMessageShifter->fromText("Use ColorShifters");
    texMessagePainter->fromText("Use Painters");
    texMessageLeveler->fromText("Use Levelers");

    texMessageMatch->fromText("Balls to match:");
    texMessageWidth->fromText("Playing area width:");

    constructMessage(MESSAGETYPE_RAND);
    constructMessage(MESSAGETYPE_PRES);
    constructMessage(MESSAGETYPE_MATCH);

    texPageTitle = new BaseTexture();
    texPageTitle ->fromText( title,gameSettings->colors[COLOR_PAGE_TITLE],glbFont);

}

//----------------------------------------------------------------------------------
int GamePlayPage::getColumnOffset(int column)
{
    return (rc.w -2*buttonWidth)/4. + column*rc.w/2.;
}

//-----------------------------------------------------------------------------------
void GamePlayPage::initControls()
{
    const char* strButtons[NUM_GAMEPLAY_BUTTONS]=
    { "Friendly","Tricky","Challenge","True Random","Peaceful","Arcade","Haste","Chaos",
    (*useAimingAid)?"Yes":"No",(*useBombs)?"Yes":"No",(*useChrome)?"Yes":"No",(*useShifter)?"Yes":"No",
    (*usePainter)?"Yes":"No",(*useLeveler)?"Yes":"No","<",">","8","10","12","14","16"};

    buttonWidth=0.4*guiBox->w;

    buttonVector.clear();

    for (int i=0;i <= ID_BTN_PRES_HIGHEST;i++)
    {
        buttonVector.push_back( new SimpleButton(texButtons[i/4],
                                          getColumnOffset(i/4),
                                          1.1*guiUnit*(i%4)+guiUnit,
                                          buttonWidth,
                                          guiUnit,
                                          strButtons[i],
                                          BT_PUSHONLY,
                                          SIMPLE_BUTTON_ALPHA)
                               );
    }

    float yOffset=7.6*guiUnit;
    for (int i=ID_BTN_USE_AIM;i<ID_BTN_LOWER;i++)
    {
        buttonVector.push_back( new SimpleButton(texButtons[2],
                                              getColumnOffset(1)+buttonWidth/2,
                                              yOffset +(i-ID_BTN_USE_AIM)*(int)guiUnit,
                                              buttonWidth/2,
                                              guiUnit*0.75,
                                              strButtons[i],
                                              BT_PUSHBUTTON,
                                              SIMPLE_BUTTON_ALPHA)
                               );
        //yOffset+=guiUnit;
    }
    buttonVector[ID_BTN_USE_AIM]->setState((*useAimingAid)?BUTTONSTATE_PUSHEDIN:BUTTONSTATE_IDLE);
    buttonVector[ID_BTN_USE_BOMBS]->setState((*useBombs)?BUTTONSTATE_PUSHEDIN:BUTTONSTATE_IDLE);
    buttonVector[ID_BTN_USE_CHROME]->setState((*useChrome)?BUTTONSTATE_PUSHEDIN:BUTTONSTATE_IDLE);
    buttonVector[ID_BTN_USE_SHIFTER]->setState((*useShifter)?BUTTONSTATE_PUSHEDIN:BUTTONSTATE_IDLE);
    buttonVector[ID_BTN_USE_PAINTER]->setState((*usePainter)?BUTTONSTATE_PUSHEDIN:BUTTONSTATE_IDLE);
    buttonVector[ID_BTN_USE_LEVELER]->setState((*useLeveler)?BUTTONSTATE_PUSHEDIN:BUTTONSTATE_IDLE);

    yOffset+=(ID_BTN_LOWER-ID_BTN_USE_AIM)*(int)guiUnit;

    for (int i=ID_BTN_LOWER;i<=ID_BTN_HIGHER;i++)
    {
        buttonVector.push_back( new SimpleButton(texButtons[3],
                                              getColumnOffset(1)+buttonWidth/2+(i-ID_BTN_LOWER)*2*buttonWidth/6,
                                              yOffset,
                                              buttonWidth/6,
                                              guiUnit*0.75,
                                              strButtons[i],
                                              BT_PRESSONLY,
                                              SIMPLE_BUTTON_ALPHA)
                               );
    }
    yOffset+=(int)guiUnit;
    int xOffset=getColumnOffset(1)+buttonWidth-(1+ID_BTN_SIZE16-ID_BTN_SIZE8)*1.1*buttonWidth/6;

    for (int i=ID_BTN_SIZE8 ;i<=ID_BTN_SIZE16;i++)
    {
        buttonVector.push_back( new SimpleButton(texButtons[3],
                                              xOffset+(i-ID_BTN_SIZE8)*1.1*buttonWidth/6,
                                              yOffset,
                                              buttonWidth/6,
                                              guiUnit*0.75,
                                              strButtons[i],
                                              BT_PUSHONLY,
                                              SIMPLE_BUTTON_ALPHA)
                               );
    }

    for (int i=0;i<NUM_GAMEPLAY_BUTTONS;i++)
        buttonVector[i]->setOnScrollPage();

    setButtonStates();
    setMatchButtonStates();
    setWellWidthButtonStates();

}

//--------------------------------------------------------------------------------

GamePlayPage::~GamePlayPage()
{
    for (int i=0; i<NUM_GAMEPLAY_BUTTONS;i++)
    {
        delete buttonVector[i];
        buttonVector[i]=NULL;
    }
    buttonVector.clear();

    delete texMessageRand;
    delete texMessagePres;
    delete texMessageAim;
    delete texMessageBombs;
    delete texMessageChrome;
    delete texMessageShifter;
    delete texMessageLeveler;
    delete texMessagePainter;
    delete texMessageMatch;
    delete texMessageWidth;
    delete texMatchNumber;

    delete texPageTitle;

    for (int i=0;i<4;i++)
        texButtons[i]=NULL;//texture manager deletes them

    texMsgBackground = NULL;

    randomness=NULL;
    pressure=NULL;

}

//-----------------------------------------------------------------

bool GamePlayPage::handleEvents()
{
    int tempRandomness, tempPressure;

    bool tempUseAid=*useAimingAid;
    bool tempUseBombs=*useBombs;
    bool tempUseChrome=*useChrome;
    bool tempUseShifter=*useShifter;
    bool tempUsePainter=*usePainter;
    bool tempUseLeveler=*useLeveler;

    int tempMatch= *matchNumber;

    tempRandomness=*randomness;
    tempPressure=*pressure;

    bool handled =false;

    for (int i=0;i<NUM_GAMEPLAY_BUTTONS;i++)
        handled |= buttonVector[i]->handleEvents(position);

    for ( int i=0;i<=ID_BTN_PRES_HIGHEST;i++)
        if (buttonVector[i]->getState()==BUTTONSTATE_PUSHEDHOVER)
        {
            //Mix_PlayChannel( -1, sndClick, 0 );

            switch(i)
            {
                case ID_BTN_RAND_LOW:   *randomness=RANDOMNESS_LOW; break;

                case ID_BTN_RAND_MODERATE: *randomness=RANDOMNESS_MODERATE;break;

                case ID_BTN_RAND_HIGH:   *randomness=RANDOMNESS_HIGH;break;

                case ID_BTN_RAND_HIGHEST:   *randomness=RANDOMNESS_HIGHEST;break;

                case ID_BTN_PRES_NONE:*pressure= PRESSURE_NONE;break;

                case ID_BTN_PRES_MODERATE: *pressure= PRESSURE_MODERATE; break;

                case ID_BTN_PRES_HIGH:  *pressure= PRESSURE_HIGH; break;

                case ID_BTN_PRES_HIGHEST:  *pressure= PRESSURE_HIGHEST; break;

                default:break;
            };

            setButtonStates();

           break;
        }

    *useAimingAid = buttonVector[ID_BTN_USE_AIM]->isPushedIn();
    *useBombs = buttonVector[ID_BTN_USE_BOMBS]->isPushedIn();
    *useChrome = buttonVector[ID_BTN_USE_CHROME]->isPushedIn();
    *useShifter = buttonVector[ID_BTN_USE_SHIFTER]->isPushedIn();
    *usePainter = buttonVector[ID_BTN_USE_PAINTER]->isPushedIn();
    *useLeveler = buttonVector[ID_BTN_USE_LEVELER]->isPushedIn();

    if (buttonVector[ID_BTN_LOWER]->gotPressed())
    {
        *matchNumber = std::max(2,*matchNumber-1);

        setMatchDownButtonState();

        if (buttonVector[ID_BTN_LOWER]->isEnabled())
             buttonVector[ID_BTN_LOWER]->setState(BUTTONSTATE_IDLE);

        buttonVector[ID_BTN_HIGHER]->enable();

        handled=true;
    }
    if (buttonVector[ID_BTN_HIGHER]->gotPressed())
    {
        *matchNumber = std::min(6,*matchNumber+1);

        setMatchUpButtonState();

        buttonVector[ID_BTN_LOWER]->enable();

        if (buttonVector[ID_BTN_HIGHER]->isEnabled())
            buttonVector[ID_BTN_HIGHER]->setState(BUTTONSTATE_IDLE);
        handled=true;
    }

    for ( int i=ID_BTN_SIZE8;i<=ID_BTN_SIZE16;i++)
        if (buttonVector[i]->getState()==BUTTONSTATE_PUSHEDHOVER)
        {
            *wellWidthID = i-ID_BTN_SIZE8;
            setWellWidthButtonStates();
            break;
        }


    if (*randomness!=tempRandomness)
        constructMessage(MESSAGETYPE_RAND);

    if (*pressure!=tempPressure)
        constructMessage(MESSAGETYPE_PRES);

    if (*useAimingAid!=tempUseAid)
        buttonVector[ID_BTN_USE_AIM]->setCaption((*useAimingAid)?"Yes":"No");
    if (*useBombs!=tempUseBombs)
        buttonVector[ID_BTN_USE_BOMBS]->setCaption((*useBombs)?"Yes":"No");
    if (*useChrome!=tempUseChrome)
        buttonVector[ID_BTN_USE_CHROME]->setCaption((*useChrome)?"Yes":"No");
    if (*useShifter!=tempUseShifter)
        buttonVector[ID_BTN_USE_SHIFTER]->setCaption((*useShifter)?"Yes":"No");
    if (*usePainter!=tempUsePainter)
        buttonVector[ID_BTN_USE_PAINTER]->setCaption((*usePainter)?"Yes":"No");
    if (*useLeveler!=tempUseLeveler)
        buttonVector[ID_BTN_USE_LEVELER]->setCaption((*useLeveler)?"Yes":"No");

    if (*matchNumber!=tempMatch)
        constructMessage(MESSAGETYPE_MATCH);


    return handled;

}
//-----------------------------------------------
void GamePlayPage::setWellWidthButtonStates()
{
    bool pushed=false;
    for (int i=ID_BTN_SIZE8;i<=ID_BTN_SIZE16;i++)
    {

        if ( ( i>ID_BTN_SIZE8 ) &&( !playerStats->getUnlockableStatus(UNL_DIM10+i-ID_BTN_SIZE10) ) )
            buttonVector[i]->disable();
        else
        {
            if (*wellWidthID == (i-ID_BTN_SIZE8))
            {
                buttonVector[i]->setState(BUTTONSTATE_PUSHEDIN);
                pushed=true;
            }
            else
                buttonVector[i]->setState(BUTTONSTATE_IDLE);
        }
    }

    if (!pushed)
        buttonVector[ID_BTN_SIZE8]->setState(BUTTONSTATE_PUSHEDIN);

}

//-----------------------------------------------
void GamePlayPage::setMatchButtonStates()
{
    setMatchUpButtonState();
    setMatchDownButtonState();
}
//-----------------------------------------------
void GamePlayPage::setMatchDownButtonState()
{
    if (*matchNumber==2)
        buttonVector[ID_BTN_LOWER]->disable();
}
//-----------------------------------------------
void GamePlayPage::setMatchUpButtonState()
{

    int nextmatch=std::min(6,*matchNumber+1);
    if (nextmatch>3)
    {
        if ( (*matchNumber==6) ||(!playerStats->getUnlockableStatus(UNL_MATCH4 +(nextmatch-4) )) )
            buttonVector[ID_BTN_HIGHER]->disable();
    }
}
//-----------------------------------------------
void GamePlayPage::constructMessage(int type)
{
    std::string str;
    if (type==MESSAGETYPE_RAND)
    {
        switch (*randomness){
            case RANDOMNESS_LOW:        str=" 10% chance the\n next ball will be\n of random color";break;
            case RANDOMNESS_MODERATE:   str=" 30% chance the\n next ball will be\n of random color";break;
            case RANDOMNESS_HIGH:       str=" 60% chance the\n next ball will be\n of random color";break;
            case RANDOMNESS_HIGHEST:    str=" 100% chance the\n next ball will be\n of random color";break;
        };
        texMessageRand->fromText(str,
                              glbTextColor,glbMessageFont,4*guiUnit);
    }
    else if (type==MESSAGETYPE_PRES)
    {
        switch (*pressure){
            case PRESSURE_NONE:     str=" Aiming timer: off\n Level Press: off\n Ball speed: normal";break;
            case PRESSURE_MODERATE: str=" Aiming timer: on\n Level Press: on\n Ball speed: quick";break;
            case PRESSURE_HIGH:     str=" Aiming timer: short\n Level Press: quick\n Ball speed: fast";break;
            case PRESSURE_HIGHEST:  str=" Aiming timer: blink\n Level Press: full on\n Ball speed: fast";break;
        };
        texMessagePres->fromText(str,
                              glbTextColor,glbMessageFont,4*guiUnit);
    }
    else if (type==MESSAGETYPE_MATCH)
    {
        char num[2];
        sprintf(num,"%d",*matchNumber);

        texMatchNumber->fromText(num,glbTextColor,glbFont);
    }

}
//-------------------------------------------------------------------------

void GamePlayPage::render(float alphaModifier)
{
    //renderTitle(alphaModifier);

    if (!isVisible()) return;
    renderTitle(alphaModifier);

    glbTexManager->getTexturePtr(imageHandle)->render(position.x,position.y,&rc,-1,alphaModifier*50);


    for (int i=0;i<NUM_GAMEPLAY_BUTTONS;i++)
        buttonVector[i]->render( position,alphaModifier);


    SDL_Rect rcMsg={0,0,(int)buttonWidth, (int)(guiUnit*1.75)};

    int messageY=position.y + 5.4*guiUnit;

    texMsgBackground->render( position.x + getColumnOffset(0),
                               messageY,&rcMsg,-1,alphaModifier*50);//70);

    texMessageRand->render(  position.x + getColumnOffset(0),
                              messageY,&rcMsg,-1,alphaModifier*255);

    texMsgBackground->render( position.x + getColumnOffset(1),
                              messageY,&rcMsg,-1,alphaModifier*50);//70);

    texMessagePres->render( position.x + getColumnOffset(1),
                            messageY,&rcMsg,-1,alphaModifier*255);

    rcMsg.x=position.x + getColumnOffset(0);
    rcMsg.y=position.y + 7.6*guiUnit;

    rcMsg.w=getColumnOffset(1)-getColumnOffset(0);
    rcMsg.h=guiUnit*0.75;

    texMsgBackground->render( rcMsg.x,rcMsg.y,&rcMsg,-1,alphaModifier*50);

    texMessageAim ->render( position.x + getColumnOffset(0)+guiUnit/8,
                            rcMsg.y,NULL,-1,alphaModifier*255);
    rcMsg.y+=(int)guiUnit;
    texMsgBackground->render( rcMsg.x,rcMsg.y,&rcMsg,-1,alphaModifier*50);

    texMessageBombs ->render( position.x + getColumnOffset(0)+guiUnit/8,
                            rcMsg.y,NULL,-1,alphaModifier*255);

    rcMsg.y+=(int)guiUnit;
    texMsgBackground->render( rcMsg.x,rcMsg.y,&rcMsg,-1,alphaModifier*50);

    texMessageChrome ->render( position.x + getColumnOffset(0)+guiUnit/8,
                            rcMsg.y,NULL,-1,alphaModifier*255);


    rcMsg.y+=(int)guiUnit;
    texMsgBackground->render( rcMsg.x,rcMsg.y,&rcMsg,-1,alphaModifier*50);

    texMessageShifter ->render( position.x + getColumnOffset(0)+guiUnit/8,
                            rcMsg.y,NULL,-1,alphaModifier*255);
    rcMsg.y+=(int)guiUnit;
    texMsgBackground->render( rcMsg.x,rcMsg.y,&rcMsg,-1,alphaModifier*50);

    texMessagePainter ->render( position.x + getColumnOffset(0)+guiUnit/8,
                            rcMsg.y,NULL,-1,alphaModifier*255);
    rcMsg.y+=(int)guiUnit;
    texMsgBackground->render( rcMsg.x,rcMsg.y,&rcMsg,-1,alphaModifier*50);

    texMessageLeveler ->render( position.x + getColumnOffset(0)+guiUnit/8,
                            rcMsg.y,NULL,-1,alphaModifier*255);



    rcMsg.y+=(int)guiUnit;
    texMsgBackground->render( rcMsg.x,rcMsg.y,&rcMsg,-1,alphaModifier*50);

    texMessageMatch ->render( position.x + getColumnOffset(0)+guiUnit/8,
                            rcMsg.y,NULL,-1,alphaModifier*255);

    rcMsg.y+=(int)guiUnit;
    texMsgBackground->render( rcMsg.x,rcMsg.y,&rcMsg,-1,alphaModifier*50);

    texMessageWidth ->render( position.x + getColumnOffset(0)+guiUnit/8,
                            rcMsg.y,NULL,-1,alphaModifier*255);

     rcMsg.y-=(int)guiUnit;
    rcMsg.x=position.x + getColumnOffset(1)+11*buttonWidth/16;
    rcMsg.w=buttonWidth/8;
    rcMsg.h=guiUnit*0.75;
    texMsgBackground->render( rcMsg.x,rcMsg.y,&rcMsg,-1,alphaModifier*50);

    texMatchNumber ->render( rcMsg.x+(rcMsg.w-texMatchNumber->getWidth())/2,
                            rcMsg.y+(rcMsg.h-texMatchNumber->getHeight())/2.,
                            NULL,-1,alphaModifier*255);

}
//------------------------------------------------------------------------------

void GamePlayPage::setButtonStates()
{
    for (int i=0;i<=ID_BTN_PRES_HIGHEST;i++)
    {
        if ( (i==ID_BTN_RAND_LOW + *randomness) ||
             (i==ID_BTN_PRES_NONE + *pressure) )
                buttonVector[i]->setState(BUTTONSTATE_PUSHEDIN);
        else buttonVector[i]->setState(BUTTONSTATE_IDLE);
    }
    if (!playerStats->getUnlockableStatus(UNL_TRUERANDOM))
        buttonVector[ID_BTN_RAND_HIGHEST]->disable();

    if (!playerStats->getUnlockableStatus(UNL_CHAOS))
        buttonVector[ID_BTN_PRES_HIGHEST]->disable();

}

//=================================================================================================

SkinPage::SkinPage(std::string titleStr, const FPoint& pos, const SDL_Rect& rect,int* skinPtr, bool page2 )
{

    skinType = skinPtr;
    isPage2=page2;

    initPage(titleStr,pos,rect,255);

    loadImages();
    initControls();
}
//-----------------------------------------------------------------------------------

SkinPage::~SkinPage()
{

    skinType=NULL;//refers to a live variable

    for (int i=0;i<4;i++)
    {
        delete buttonVector[i];
        buttonVector[i]=NULL;

        texButtons[i]=NULL;//texturemanager handles it
    }
    buttonVector.clear();
}
//--------------------------------------------------------------------------

void SkinPage::loadImages()
{

    imageHandle = glbTexManager->insertTexture("page_background.png",SKIN_UNIVERSAL,true);

    if (isPage2)
    {
        texButtons[SKIN_DARK]=glbTexManager->getTexturePtr(glbTexManager->insertTexture("thumbs/skin_dark2.png",SKIN_UNIVERSAL,true));
        texButtons[SKIN_LIGHT]=glbTexManager->getTexturePtr(glbTexManager->insertTexture("thumbs/skin_light2.png",SKIN_UNIVERSAL,true));
        texButtons[SKIN_PATTERN]=glbTexManager->getTexturePtr(glbTexManager->insertTexture("thumbs/skin_pattern2.png",SKIN_UNIVERSAL,true));
        texButtons[SKIN_GEOMETRIC]=glbTexManager->getTexturePtr(glbTexManager->insertTexture("thumbs/skin_geometric2.png",SKIN_UNIVERSAL,true));
    }
    else
    {
        texButtons[SKIN_DARK]=glbTexManager->getTexturePtr(glbTexManager->insertTexture("thumbs/skin_dark.png",SKIN_UNIVERSAL,true));
        texButtons[SKIN_LIGHT]=glbTexManager->getTexturePtr(glbTexManager->insertTexture("thumbs/skin_light.png",SKIN_UNIVERSAL,true));
        texButtons[SKIN_PATTERN]=glbTexManager->getTexturePtr(glbTexManager->insertTexture("thumbs/skin_pattern.png",SKIN_UNIVERSAL,true));
        texButtons[SKIN_GEOMETRIC]=glbTexManager->getTexturePtr(glbTexManager->insertTexture("thumbs/skin_geometric.png",SKIN_UNIVERSAL,true));

    }

    texPageTitle = new BaseTexture();
    texPageTitle ->fromText( title,gameSettings->colors[COLOR_PAGE_TITLE],glbFont);

}

//--------------------------------------------------------

void SkinPage::setButtonStates()
{
    for (int i=0;i<4;i++)
    {
        if (buttonVector[i]->isEnabled())
        {

            if (*skinType== i+ (isPage2?4:0))
            {
                buttonVector[i]->setState(BUTTONSTATE_PUSHEDIN);
            }
            else
            {
                buttonVector[i]->setState(BUTTONSTATE_IDLE);
            }
        }
    }

}

//--------------------------------------------------------------

void SkinPage::initControls()
{

    const char* captions[8]={"Dark","Light","Patterned","Geometric","Dark2","Light2","Patterned2","Geometric2"};

    offset= guiUnit/2.;

    buttonWidth=(rc.w-3*offset)/2;
    buttonHeight=(rc.h-guiUnit-2*offset)/2;

    buttonVector.clear();

    for (int i=0;i<4;i++)
    {
        buttonVector.push_back( new GlassButton(texButtons[i],
                                                offset+(offset+buttonWidth)*(i>>1),// i/2
                                                guiUnit+(offset+buttonHeight)*(i&1),//i%2
                                                buttonWidth,buttonHeight,captions[i + (isPage2?4:0) ],BT_PUSHONLY,
                                                (GLASS_BUTTON_ALPHA+255)/2 )
                               );
        buttonVector[i]->setOnScrollPage();

        if (isPage2 && (!playerStats->getUnlockableStatus(UNL_SKIN_DARK2+i)))
            buttonVector[i]->disable();
    }
    setButtonStates();

}

//-------------------------------------------------------------------------

void SkinPage::render(float alphaModifier)
{

    if (!isVisible()) return;

    renderTitle();

    glbTexManager->getTexturePtr(imageHandle)->render(position.x,position.y,&rc,-1,alphaModifier*50);

    for (int i=0;i<buttonVector.size();i++)
        buttonVector[i]->render( position,alphaModifier);
}

//------------------------------------------------------------------------------
//------------------------------------------------

bool SkinPage::handleEvents()
{
    int tempSkin = *skinType;

    bool handled =false;

    for (int i=0;i<buttonVector.size();i++)
        handled |= buttonVector[i]->handleEvents(position);

    if (handled)
    {
        for ( int i=0;i<buttonVector.size();i++)
            if (buttonVector[i]->getState()==BUTTONSTATE_PUSHEDHOVER)
            {
                //Mix_PlayChannel( -1, sndClick, 0 );
                *skinType=i+(isPage2?4:0);
                break;
            }

        if (*skinType!=tempSkin)
        {
            setButtonStates();
        }
    }

    return handled;

}
//===================================================================================

EffectsPage::EffectsPage(std::string titleStr, const FPoint& pos, const SDL_Rect& rect,bool *bgEffectPtr, bool *translucencyPtr, bool *explodePtr, bool *shinePtr, bool *complexityPtr, bool *trailsPtr, bool *flashPtr, int *musicVolume, int* sfxVolume)
{
    settings[ SETTING_RENDER_BGEFFECT ] = bgEffectPtr ;
    settings[ SETTING_TRANSLUCENT_BGS ] = translucencyPtr;
    settings[ SETTING_MULTI_EXPLODE ]  = explodePtr;
    settings[ SETTING_SHINE_EFFECT ]    = shinePtr;
    settings[ SETTING_FULL_COMPLEXITY ] = complexityPtr;
    settings[ SETTING_TRAILS ]          = trailsPtr;
    settings[ SETTING_FLASH ]            = flashPtr;

    volumeLevel[0] = musicVolume;
    volumeLevel[1] = sfxVolume;

    initPage(titleStr,pos,rect,255);

    loadImages();

    initControls();
}
//----------------------------------------------------------
EffectsPage::~EffectsPage()
{
    for (int i=0; i<NUM_EFFECT_SETTINGS;i++)
    {
        delete buttonVector[i];
        buttonVector[i]=NULL;

        settings[i]=NULL;
    }

    for (int i=0; i<NUM_EFFECT_SETTINGS+2;i++)
    {
        delete texMessages[i];
        texMessages[i]=NULL;
    }


    buttonVector.clear();

    for (int i=0; i<2;i++)
    {
        delete volumeSlider[i];
        volumeSlider[i]=NULL;
    }

    delete texPageTitle;

    texButtons=NULL;//texture manager deletes them

    texMsgBackground = NULL;

}
//----------------------------------------------------------
int EffectsPage::getColumnOffset(int column)
{
    return (rc.w -2*buttonWidth)/4. + column*rc.w/2.;

}
//----------------------------------------------------------
void EffectsPage::initControls()
{
    buttonWidth=0.4*guiBox->w;

    buttonVector.clear();

    int offset=getColumnOffset(1)+buttonWidth/2;

    for (int i=0;i<NUM_EFFECT_SETTINGS;i++)
    {
        buttonVector.push_back( new SimpleButton(texButtons,
                                              offset,
                                              guiUnit*1.1*(i+1),
                                              buttonWidth/2,
                                              guiUnit*0.75,
                                              (*settings[i])?"Yes":"No",
                                              BT_PUSHBUTTON,
                                              SIMPLE_BUTTON_ALPHA)
                               );
        buttonVector[i]->setState((*settings[i])?BUTTONSTATE_PUSHEDIN:BUTTONSTATE_IDLE);
        buttonVector[i]->setOnScrollPage();

        rects[i]={(int)(getColumnOffset(0)),
                    (int)(guiUnit*(1+1.1*i)),
                    (int)getColumnOffset(1)-getColumnOffset(0)+buttonWidth/4,
                     (int)(0.95*guiUnit)
                     };
    }


    for (int i=0;i<2;i++)
    {
        float v= *volumeLevel[i]*1./MIX_MAX_VOLUME;
        v= acos(1.-v)/(PI*0.5);

        volumeSlider[i]= new BaseSlider( getColumnOffset(0),(10+i*2)*guiUnit,8.8*guiUnit,guiUnit,HORIZONTAL,v);
        volumeSlider[i]->setOnScrollPage();

        rects[NUM_EFFECT_SETTINGS+i]={  (int)(getColumnOffset(0)),
                                        (int)(guiUnit*(10+2*i)-texMessages[NUM_EFFECT_SETTINGS+i]->getHeight()),
                                         texMessages[NUM_EFFECT_SETTINGS+i]->getWidth()+guiUnit/4,
                                         texMessages[NUM_EFFECT_SETTINGS+i]->getHeight()
                                         };

    }


}
//----------------------------------------------------------
void EffectsPage::loadImages()
{

    imageHandle = glbTexManager->insertTexture("page_background.png",SKIN_UNIVERSAL,true);

    texMsgBackground = glbTexManager->getTexturePtr(
                        glbTexManager->insertTexture("message_background.png",SKIN_UNIVERSAL,true));

    texMsgBackground->useAlphaBlending(true);


    texButtons = glbTexManager->getTexturePtr(
                        glbTexManager->insertTexture("buttons/buttons_yellow.png",SKIN_UNIVERSAL,true));

    const char* messageTexts[NUM_EFFECT_SETTINGS+2]= {
                                                    "Render translucent backgrounds\n",
                                                    "Render background effect during play\n",
                                                    "Render Shine effect following your touch\n",
                                                    "Render explosions on power balls collision\n",
                                                    "Render motion trails - enhances all effects\n",
                                                    "Use larger and more detailed effects\n",
                                                    "Screen Flash on special events\n",
                                                    "Music Volume",
                                                    "Sound Effects Volume"
                                                    };

    for (int i=0;i<NUM_EFFECT_SETTINGS+2;i++)
    {
        texMessages[i]= new BaseTexture();
        texMessages[i]->fromText( messageTexts[i],glbTextColor,glbMessageFont,4.5*guiUnit);
    }

    texPageTitle = new BaseTexture();
    texPageTitle ->fromText( title,gameSettings->colors[COLOR_PAGE_TITLE],glbFont);

}
//----------------------------------------------------------
void EffectsPage::render(float alphaModifier )
{
    //renderTitle(alphaModifier);

    if (!isVisible()) return;

    renderTitle(alphaModifier);

    glbTexManager->getTexturePtr(imageHandle)->render(position.x,position.y,&rc,-1,alphaModifier*50);


    for (int i=0;i<NUM_EFFECT_SETTINGS;i++)
        buttonVector[i]->render( position,alphaModifier);

    for (int i=0;i<NUM_EFFECT_SETTINGS+2;i++)
    {
        texMsgBackground->render( position.x+rects[i].x,
                                 position.y+rects[i].y,&rects[i],-1,alphaModifier*50);//80

        texMessages[i]->render( position.x+rects[i].x+guiUnit/8,
                                position.y+rects[i].y, NULL,-1,alphaModifier*255);
    }

    for (int i=0;i<2;i++)
        volumeSlider[i]->render(position,alphaModifier);

}
//----------------------------------------------------------
bool EffectsPage::handleEvents()
{
    bool temp[NUM_EFFECT_SETTINGS];

    for (int i=0;i<NUM_EFFECT_SETTINGS;i++)
        temp[i]=*settings[i];

    bool handled =false;

    for (int i=0;i<NUM_EFFECT_SETTINGS;i++)
        handled |= buttonVector[i]->handleEvents(position);

    for (int i=0;i<2;i++)
    {
        handled|=volumeSlider[i]->handleEvents(position);

        if( volumeSlider[i]->levelChanged())
        {
            float v=volumeSlider[i]->getLevel();

            *volumeLevel[i]=MIX_MAX_VOLUME*(1-myCosine(v*PI*0.5));

            if (i==0)
                setMusicVolume(*volumeLevel[i]);
            else
            {
                setSFXVolume(*volumeLevel[i]);
                playSound(SFX_BUBBLE,0);
            }
        }
    }


    for (int i=0;i<NUM_EFFECT_SETTINGS;i++)
    {
        *settings[i]= buttonVector[ i ]->isPushedIn();
        if (*settings[i]!=temp[i])
        {
            buttonVector[ i ]->setCaption((*settings[i])?"Yes":"No");
        }
    }

    return handled;
}
//----------------------------------------------------------
//===================================================================================

InfoPage::InfoPage(std::string titleStr, const FPoint& pos, const SDL_Rect& rect,bool *scorePtr, bool* timePtr, bool *shotsPtr, bool *bestPtr, bool *statsPtr, bool *beatenPtr, bool *swipePtr, bool *overPtr)
{
    settings[ SETTING_SCORE ] = scorePtr;
    settings[ SETTING_TIME ] = timePtr;
    settings[ SETTING_SHOTS ] = shotsPtr;
    settings[ SETTING_BEST ] = bestPtr;
    settings[ SETTING_STATS ] = statsPtr;
    settings[ SETTING_BEATEN ] = beatenPtr;
    settings[ SETTING_SWIPEHINT] = swipePtr;
    settings[ SETTING_OVERRIDE] = overPtr;

    initPage(titleStr,pos,rect,255);

    loadImages();

    initControls();
}
//----------------------------------------------------------
InfoPage::~InfoPage()
{
    for (int i=0; i<NUM_INFO_SETTINGS;i++)
    {
        delete buttonVector[i];
        buttonVector[i]=NULL;

        delete texMessages[i];
        texMessages[i]=NULL;

        settings[i]=NULL;
    }
    buttonVector.clear();

    delete texPageTitle;

    texButtons=NULL;//texture manager deletes them

    texMsgBackground = NULL;
}
//----------------------------------------------------------
int InfoPage::getColumnOffset(int column)
{
    return (rc.w -2*buttonWidth)/4. + column*rc.w/2.;

}
//----------------------------------------------------------
void InfoPage::initControls()
{



    buttonVector.clear();

    buttonWidth=0.4*guiBox->w;
    int offset=getColumnOffset(1)+buttonWidth/2;

    for (int i=0;i<NUM_INFO_SETTINGS;i++)
    {
        buttonVector.push_back( new SimpleButton(texButtons,
                                              offset,
                                              guiUnit*1.1*(i+1),
                                              buttonWidth/2,
                                              guiUnit*0.75,
                                              (*settings[i])?"Yes":"No",
                                              BT_PUSHBUTTON,
                                              SIMPLE_BUTTON_ALPHA)
                               );
        buttonVector[i]->setState((*settings[i])?BUTTONSTATE_PUSHEDIN:BUTTONSTATE_IDLE);
        buttonVector[i]->setOnScrollPage();

        rects[i]={(int)(getColumnOffset(0)),
                    (int)(guiUnit*(1+1.1*i)),
                    (int)getColumnOffset(1)-getColumnOffset(0)+buttonWidth/4,
                     (int)(0.95*guiUnit)
                     };
        rects[SETTING_OVERRIDE].h*=1.5;
    }

}
//----------------------------------------------------------
void InfoPage::loadImages()
{

    imageHandle = glbTexManager->insertTexture("page_background.png",SKIN_UNIVERSAL,true);

    texMsgBackground = glbTexManager->getTexturePtr(
                        glbTexManager->insertTexture("message_background.png",SKIN_UNIVERSAL,true));

    texMsgBackground->useAlphaBlending(true);


    texButtons = glbTexManager->getTexturePtr(
                        glbTexManager->insertTexture("buttons/buttons_blue.png",SKIN_UNIVERSAL,true));


    const char* messageTexts[NUM_INFO_SETTINGS]= {
                                                    "Show round score while playing\n",
                                                    "Show elapsed round time while playing\n",
                                                    "Show fired round shots while playing\n",
                                                    "Show best round scores while playing\n",
                                                    "Show stats in pause screen\n",
                                                    "Show stars for beaten stat limits\n",
                                                    "Show \'Swipe to Scroll\' hint\n",
                                                    "Allow [Continue] to set game settings to their old saved values\n"
                                                    };

    for (int i=0;i<NUM_INFO_SETTINGS;i++)
    {
        texMessages[i]= new BaseTexture();
        texMessages[i]->fromText( messageTexts[i],glbTextColor,glbMessageFont,4.5*guiUnit);
    }

    texPageTitle = new BaseTexture();
    texPageTitle ->fromText( title,gameSettings->colors[COLOR_PAGE_TITLE],glbFont);

}
//----------------------------------------------------------
void InfoPage::render(float alphaModifier )
{

    if (!isVisible()) return;

    renderTitle(alphaModifier);
    glbTexManager->getTexturePtr(imageHandle)->render(position.x,position.y,&rc,-1,alphaModifier*50);


    for (int i=0;i<NUM_INFO_SETTINGS;i++)
    {
        buttonVector[i]->render( position,alphaModifier);

        texMsgBackground->render( position.x+rects[i].x,
                                 position.y+rects[i].y,&rects[i],-1,alphaModifier*50);

        texMessages[i]->render( position.x+rects[i].x+guiUnit/8,
                                position.y+rects[i].y, NULL,-1,alphaModifier*255);
    }


}
//----------------------------------------------------------
bool InfoPage::handleEvents()
{
    bool temp[NUM_INFO_SETTINGS];

    for (int i=0;i<NUM_INFO_SETTINGS;i++)
        temp[i]=*settings[i];

    bool handled =false;

    for (int i=0;i<NUM_INFO_SETTINGS;i++)
        handled |= buttonVector[i]->handleEvents(position);

    for (int i=0;i<NUM_INFO_SETTINGS;i++)
    {
        *settings[i]= buttonVector[ i ]->isPushedIn();
        if (*settings[i]!=temp[i])
        {
            buttonVector[ i ]->setCaption((*settings[i])?"Yes":"No");
        }
    }

    return handled;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
