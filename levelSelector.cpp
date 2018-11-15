#include "base.h"
#include "basefuncs.h"
#include "texture.h"
#include "button.h"
#include "levelStats.h"
#include "gameSettings.h"
#include "gameMenu.h"//for  actions enumeration
#include "levelSelector.h"

#include <cmath>
#include <cstdio>

const int NUM_LEVELSELECT_BUTTONS=9;

LevelPage::LevelPage(std::string titleStr, const FPoint& pos, const SDL_Rect& rect, int* levelPtr,int levelMax)
{
    pageID=-1;
    dim=sqrt(NUM_LEVELSELECT_BUTTONS);

    initPage(titleStr,pos,rect,255);

    maxLevel=levelMax;

    selectedLevel = levelPtr;

    loadImages();
    initControls();
}

//----------------------------------------------------------

LevelPage::~LevelPage()
{
    selectedLevel=NULL;

    for (int i=0;i<buttonVector.size();i++)
    {
        delete buttonVector[i];
        buttonVector[i]=NULL;
    }
    buttonVector.clear();

    if (texPageTitle!=NULL)
        delete texPageTitle;
}

//----------------------------------------------------------

int LevelPage::getButtonX(int buttonID)
{
    return offset/2+(buttonWidth+offset)*(buttonID%dim);
}

//----------------------------------------------------------

int LevelPage::getButtonY(int buttonID)
{
    return offset/2 +(buttonHeight+offset)*(buttonID/dim);
}

//----------------------------------------------------------

void LevelPage::initControls()
{
    offset = 0.15*guiUnit;
    buttonWidth = (guiBox->w -dim*offset)/(dim*1.);
    buttonHeight = ( guiBox->h  -dim*offset)/(dim*1.);

    int i;

    buttonVector.clear();
    buttonVector.resize(NUM_LEVELSELECT_BUTTONS,NULL);

    for (i=0;i<buttonVector.size();i++)
        buttonVector[i]= new LevelButton( NULL,
                                        getButtonX(i),
                                        getButtonY(i),
                                        buttonWidth,buttonHeight,
                                        "",BT_PUSHONLY,
                                        GLASS_BUTTON_ALPHA);

    for (i=0;i<buttonVector.size();i++)
    {
        buttonVector[i]->setOnScrollPage();
        ((LevelButton*)buttonVector[i])->setLevel(-1);
    }

}

//----------------------------------------------------------

void LevelPage::loadImages()
{
    texPageTitle = new BaseTexture();
    texPageTitle ->fromText( title,gameSettings->colors[COLOR_PAGE_TITLE],glbFont);
}



//----------------------------------------------------------

void LevelPage::render(float alphaModifier)
{

//    renderTitle(alphaModifier);

    if (!isVisible()) return;

    renderTitle(alphaModifier);

    for (int i=0;i<buttonVector.size();i++)
        buttonVector[i]->render( position,alphaModifier);
}

//----------------------------------------------------------


bool LevelPage::handleEvents()
{
    int i;
    bool handled=false;

    for (i=0;i<buttonVector.size();i++)
        handled |= buttonVector[i]->handleEvents(position);

    for (i=0;i<buttonVector.size();i++)
        if (buttonVector[i]->getState() == BUTTONSTATE_PUSHEDHOVER)
            *selectedLevel = ((LevelButton*)(buttonVector[i]))->getLevel();

    return handled;
}

//----------------------------------------------------------

void LevelPage::renderTitle(float alphaModifier)
{

}
//-------------------------------------------------------------
void LevelPage::setPageContent( int newID)
{
#ifdef CONSOLE_LOG
            sprintf(glbLogMsg,"LEVEL PAGE: NEW ID: %d\n",newID);
            mySDL_Log(glbLogMsg);
#endif
    int newLevel;

    for (int i=0;i<buttonVector.size();i++)
    {
        newLevel=newID*buttonVector.size() +i;

        ((LevelButton*)buttonVector[i])->setLevel( newLevel,true);

        if (newLevel>maxLevel)
            buttonVector[i]->disable();
    }
}

//=====================EO LEVELPAGE===================================


//================LEVEL SELECTOR================================
LevelSelector::LevelSelector( int startingLevel )
{
    level=-1;
    action = ACTION_PLAYLEVEL;
    done = false;

    if (startingLevel <0) startingLevel=0;

    int maxLevel=playerStats->getMaxLevel();

#ifdef DEMO_GAME
    maxLevel=std::min(maxLevel,MAX_DEMO_LEVEL);
#else
    if (!isLegitimateCopy)
        maxLevel=std::min(maxLevel,MAX_DEMO_LEVEL);
#endif // DEMO_GAME

    if (startingLevel > maxLevel)
            startingLevel = maxLevel;

    SDL_Rect hRC={0,0,guiBox->w, guiBox->h};
    FPoint pos = FPoint(guiBox->x, guiBox->y);

    int maxPage= maxLevel/NUM_LEVELSELECT_BUTTONS+1;
    int startingPage=startingLevel/NUM_LEVELSELECT_BUTTONS;

    #ifdef CONSOLE_LOG
        sprintf(glbLogMsg, "LEVEL SELECTOR: MAX page= %d STARTING PAGE= %d\n",maxPage,startingPage);
        mySDL_Log(glbLogMsg);
    #endif



    pageHandler = new ManhattanPageHandler(ManhattanRing::ROLLING_MANHATTAN,
                                           true,
                                           1 + (int)(sqrt(maxPage)),
                                           pos,hRC,70,maxPage,startingPage);
    populatePages(maxLevel);
    #ifdef CONSOLE_LOG
        sprintf(glbLogMsg, "END OF CONSTRUCTOR!!!!!!!!!!!!!!!!!!!!!!\n");
        mySDL_Log(glbLogMsg);
    #endif
}

LevelSelector::~LevelSelector()
{
    delete pageHandler;
}

void LevelSelector::populatePages(int maxLevel)
{
    SDL_Rect rec = { 0, 0, guiBox->w,guiBox->h};

    FPoint pos( guiBox->x, guiBox->y );

    for (int i=0;i<5;i++)
        pageHandler->addPage(new LevelPage("",pos,rec,&level,maxLevel));

    pageHandler->doneInserting();
}

void LevelSelector::handleEvents(SDL_Event& event)
{
    int tempLevel=level;

    pageHandler->handleEvents();

    if (level!=tempLevel)
    {
        action=ACTION_PLAYLEVEL;
        done=true;
    }
}

void LevelSelector::render(float phase)
{
    pageHandler->render(phase);
}

void LevelSelector::logic(float dt, float phase)
{
    pageHandler->update(dt,phase);
}

bool LevelSelector::isDone(int *actn, int *lvl)
{
    if ( done )
    {
        *actn = action;
        *lvl = level;
    }
    return done;
}
