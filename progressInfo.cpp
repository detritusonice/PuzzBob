

#include "base.h"

#include "basefuncs.h"

#include "stateMachine.h"
#include "states.h"
#include "progressInfo.h"

#include "texture.h"
#include "button.h"

#include "levelStats.h"
#include "gameSettings.h"

#include "screenEffects.h"

#include <cstdio> //for sprintf()
#include <cstring>

#include "myMath.h"

//=====================================================

ProgressInfo::ProgressInfo()
{
#ifdef CONSOLE_LOG
    sprintf(glbLogMsg,"ProgressInfo\n");
    mySDL_Log(glbLogMsg);
#endif

    isHibernating=false;


    texButton = glbTexManager->getTexturePtr(
                        glbTexManager->insertTexture("buttons/buttons_cyan.png",SKIN_UNIVERSAL,true));


    initButtons();

    SDL_Rect handlerRect={0,0,(int)(guiBox->w-0.1*guiUnit),(int)(guiBox->h - 1.7*guiUnit)};

    pageHandler = new AxialPageHandler(
                                       HORIZONTAL,false,
                                       FPoint( guiBox->x+ (guiBox->w-handlerRect.w)/2,
                                              guiBox->y+guiUnit/20),
                                       handlerRect,70);

    populatePages();

  //  sndClick        = Mix_LoadWAV( "media/snd_menu.wav" );

    SDL_ShowCursor(SDL_ENABLE);

}
//------------------------------------------------------
ProgressInfo::~ProgressInfo()
{
  SDL_ShowCursor(SDL_DISABLE);

 //   Mix_HaltChannel(-1);

 //   Mix_FreeChunk(sndClick);

    delete pageHandler;

    delete returnButton;
    returnButton = NULL;

    texButton=NULL;//texture manager deletes them

}
//------------------------------------------------------
void ProgressInfo::initButtons()
{
    int buttonWidth = 4*guiUnit;
    returnButton= new SimpleButton(texButton,
                                          guiBox->x + (guiBox->w-buttonWidth)/2,
                                          guiBox->y + guiBox->h - 1.5*guiUnit,
                                          buttonWidth,
                                          guiUnit,
                                          "Return",
                                          BT_PRESSONLY,
                                          SIMPLE_BUTTON_ALPHA);
}

//------------------------------------------------------
void ProgressInfo::Logic(float dt, float phase)
{
    pageHandler->update(dt,phase);
}
//------------------------------------------------------
void ProgressInfo::Render(float phase)
{
    glbEffect->render();

    pageHandler->render(phase);

    returnButton->render();
}
//------------------------------------------------------
void ProgressInfo::HandleEvents()
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

                     if (gameSettings->musicVolume>0)
                         if (isHibernating) Mix_PauseMusic();
                            else Mix_ResumeMusic();
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
        else
            {
                if (event.type==SDL_KEYDOWN)
                {
                    switch (event.key.keysym.sym)
                    {
                        case SDLK_AC_BACK:
                        case SDLK_ESCAPE: StateMachine::SetNextState(STATE_TITLE);break;
                    }
                }
                else if (event.type==SDL_MOUSEMOTION)
                {
                    glbEffect->moveEffect(FPoint(event.motion.x, event.motion.y));
                }
            }
    }

    bool handled = returnButton->handleEvents();

    if (!handled)
        pageHandler->handleEvents();

    if (returnButton->gotPressed() )
            StateMachine::SetNextState(STATE_TITLE);

}
//------------------------------------------------------
void ProgressInfo::populatePages()
{
    SDL_Rect rc = { 0, 0, (int)( guiBox->w - 0.25*guiUnit),
                         (int)(guiBox->h - 1.85*guiUnit)};

    FPoint pos( (screenRect->w-rc.w)/2. ,guiBox->y + guiUnit/8);

    //char pageTitle[30];

    int numPages= (NUM_SUMMONS-1)/6;
    if ((NUM_SUMMONS-1)%6) numPages++;

    for (int i=0;i< numPages ;i++)
        pageHandler->addPage( new ProgressInfoPage( "PowerBall Summons",pos,rc,true,1+i*6,6), true);

    numPages= NUM_UNLOCKABLES/4;
    if (NUM_UNLOCKABLES%4) numPages++;

    for (int i=0;i< numPages ;i++)
        pageHandler->addPage( new ProgressInfoPage( "Game Unlockables",pos,rc,false,i*4,4), false);


    pageHandler->doneInserting();
}

//==============================================================

ProgressInfoPage::ProgressInfoPage(std::string titleStr, const FPoint& pos, const SDL_Rect& rect, bool isSummons, int first, int maxItemNumber)
{
    firstItem = first;
    numItems = std::min( (isSummons?(int)NUM_SUMMONS:(int)NUM_UNLOCKABLES)-first, maxItemNumber);

    isSummonsPage = isSummons;
    maxItems=maxItemNumber;

    initPage(titleStr,pos,rect,255);

    getStatusStrings();

    loadImages();

    initControls();

    setMessageRects();
    //setIconRects();
}
//------------------------------------------------------
ProgressInfoPage::~ProgressInfoPage()
{
    delete texPageTitle;

    texMsgBackground = NULL;

    for (int i=0;i<numItems;i++)
    {
        delete texMsgVector[i];
        texMsgVector[i]=NULL;
        texBadgeVector[i]=NULL;
    }



}
//------------------------------------------------------
void ProgressInfoPage::getStatusStrings()
{
    strVector.clear();
    statusVector.clear();

    bool status;
    std::string s;

    for (int i=firstItem;i<firstItem+numItems;i++)
    {
        if (isSummonsPage)
            status= playerStats->getSummonString(i,&s);
        else
            status= playerStats->getUnlockableString(i,&s);

        statusVector.push_back(status);
        strVector.push_back(s);
    }
}
//------------------------------------------------------
void ProgressInfoPage::loadImages()
{
    const char* badgeNames[NUM_SUMMONS+NUM_UNLOCKABLES]={"","balls/ball_bomb.png","balls/ball_shifter.png","balls/timestop.png",
                                                    "balls/ball_painter.png","balls/ball_leveler.png","balls/ball_chrome.png",
                                                    "badges/truerandom.png","badges/chaos.png","thumbs/skin_dark2.png","thumbs/skin_light2.png",
                                                    "thumbs/skin_pattern2.png","thumbs/skin_geometric2.png","badges/match4.png","badges/match5.png","badges/match6.png",
                                                    "badges/width10.png","badges/width12.png","badges/width14.png","badges/width16.png"
#ifdef EXTRA_GAME_MODES
                                                    ,"badges/timelimit.png","badges/shotlimit.png","badges/scorelimit.png","badges/editor.png"
#endif // EXTRA_GAME_MODES
                                                   };

    imageHandle = glbTexManager->insertTexture("page_background.png",SKIN_UNIVERSAL,true);

    texMsgBackground = glbTexManager->getTexturePtr(
                        glbTexManager->insertTexture("message_background.png",SKIN_UNIVERSAL,true));

    texMsgVector.clear();

    for (int i=0;i<numItems;i++)
    {

        texMsgVector.push_back( new BaseTexture());
        texMsgVector[i]->fromText(strVector[i].c_str(),gameSettings->colors[statusVector[i]?COLOR_NOBG2:COLOR_TEXT1],glbMessageFont,0.75*rc.w);
    }
    texBadgeVector.clear();
    int firstImageID=firstItem;

    if (!isSummonsPage) firstImageID+=NUM_SUMMONS;

    for (int i=firstImageID;i< firstImageID + numItems;i++)
        texBadgeVector.push_back(glbTexManager->getTexturePtr(glbTexManager->insertTexture(badgeNames[i],SKIN_UNIVERSAL,true)) );


    texPageTitle = new BaseTexture();
    texPageTitle ->fromText( title,gameSettings->colors[COLOR_PAGE_TITLE],glbFont);
}
//------------------------------------------------------
void ProgressInfoPage::initControls()
{
    buttonVector.clear();
}

//--------------------------------------------------------
void ProgressInfoPage::setMessageRects()
{

    int offsetX=rc.w*0.2;
    int offsetY=(int) guiUnit*1.2;
    int stepY = (rc.h-offsetY)/(1.*maxItems);

    int badgeWidth =  offsetX-rc.w*0.02;
    int badgeHeight = isSummonsPage? offsetX-rc.w*0.02:stepY-rc.w*0.03;

    for (int i=0;i<numItems;i++)
    {
        msgRectVector.push_back({offsetX,(int)(offsetY+i*stepY),(int)(rc.w*0.78),(int)stepY-guiUnit/8});
        badgeRectVector.push_back({rc.w*0.01,(int)(offsetY+i*stepY+rc.w*0.01),badgeWidth,badgeHeight});
    }
}
//------------------------------------------------------
void ProgressInfoPage::render(float alphaModifier )
{

    if (!isVisible()) return;

    renderTitle(alphaModifier);

    int i;

    glbTexManager->getTexturePtr(imageHandle)->render(position.x,position.y,
                                                      &rc,-1,alphaModifier*50);

    for (i=0;i<numItems;i++)
    {
        texBadgeVector[i]->render(position.x+badgeRectVector[i].x,
                           position.y+badgeRectVector[i].y,
                           &badgeRectVector[i],-1,(statusVector[i]?255:60)*alphaModifier);

        texMsgBackground->render(position.x + msgRectVector[i].x,
                           position.y+msgRectVector[i].y,
                           &msgRectVector[i],-1,50*alphaModifier);

        texMsgVector[i]->render(position.x+msgRectVector[i].x+rc.w*0.01,
                           position.y+msgRectVector[i].y,
                           NULL,-1,255*alphaModifier);
    }
}
//------------------------------------------------------
bool ProgressInfoPage::handleEvents()
{
    return false;
}
