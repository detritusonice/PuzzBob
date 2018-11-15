
#include "base.h"
#include "summonMenu.h"
#include "myMath.h"

#include "texture.h"
#include "button.h"
#include "levelStats.h"
#include "basefuncs.h"
#include "gameSettings.h"
#include "localEffects.h"

#include <cstdio>

const int ALPHA_AVAILABLE= 100;
const int ALPHA_DISABLED = 50;
const int ALPHA_SELECTED = 255;

//===================================================================================================

SummonMenu::SummonMenu(const SDL_Rect& rcMenu, int playingLevel, bool timeStopUsed, bool ballSummoned, int ballChosen, bool powerballGenerated)
{
    done=false;
    commitChanges=false;
    itemsAvailable=false;

    rc=rcMenu;
    level= playingLevel;
    selectedItem = ballChosen;
    powerBallsAvailable=!powerballGenerated;
    timeStopAvailable=!timeStopUsed;
    ballAlreadySummoned = ballSummoned;

    loadImages();

    initButtons();

    createMessages();

    invStatic=true;
    //updateStaticImage();

    createEffects();
}

//------------------------------------------------------------------------------------------------------

SummonMenu::~SummonMenu()
{
    int i;

    for (i=0;i<NUM_CONTROL_BUTTONS;i++)
        delete controlButtons[i];

    for (i=0;i<NUM_BALL_BUTTONS;i++)
        delete ballButtons[i];

    texButtons=NULL;

    for (i=0;i<2;i++)
        if (texMessages[i]!=NULL)
            delete texMessages[i];

    for (i=0;i<NUM_BALL_BUTTONS;i++)
    {
        if (texBallMessages[i]!=NULL)
            delete texBallMessages[i];

        if (ballEffects[i]!=NULL)
            delete ballEffects[i];

        texBalls[i]=NULL;
    }
    if (texBallMessages[NUM_BALL_BUTTONS]!=NULL)
            delete texBallMessages[NUM_BALL_BUTTONS];

    if (texStatic!=NULL)
        delete texStatic;
}
//------------------------------------------------------------------------------------------------------
bool SummonMenu::createButtonMessage(int buttonID, int blocklevel)
{
    bool active=false;

    if (
        ( (buttonID == ID_BTN_BOMB) && ( !gameSettings->useBombs ) )
        ||
        ( (buttonID == ID_BTN_SHIFTER) && ( !gameSettings->useShifters ) )
        ||
        ( (buttonID == ID_BTN_PAINTER) && ( !gameSettings->usePainters ) )
        ||
        ( (buttonID == ID_BTN_LEVELER) && ( !gameSettings->useLevelers ) )
        ||
        ( (buttonID == ID_BTN_CHROME) && ( !gameSettings->useChromeBalls ) )
/*            ||
        ( (buttonID == ID_BTN_TIMESTOP) && ( !gameSettings->useTimeStop ) )
*/
       )
        {
            sprintf(ballTexts[buttonID],"Disabled");
            active=false;
        }
    else
    if ( blocklevel!=-1 )
    {
        if (!playerStats->isSummonUnlocked(buttonID+1,level))//unlockable bomb is at button 0
            sprintf(ballTexts[buttonID],"Locked (%d)",blocklevel+1);
        else
            sprintf(ballTexts[buttonID],"Used (%d)",blocklevel+1);
        active=false;
    }
    else
    {
        if (
            ( (buttonID == ID_BTN_TIMESTOP) && ( !timeStopAvailable ) )
            ||
            ( ((buttonID) >= ID_BTN_PAINTER) && ( !powerBallsAvailable) )
            ||
            ( ((buttonID) !=ID_BTN_TIMESTOP) && ( ballAlreadySummoned) )
            )
            {
                sprintf(ballTexts[buttonID],"Not Available");
                active=false;
            }
        else
            {
                sprintf(ballTexts[buttonID],"Available");
                active=true;
            }
    }
    return active;
}
//------------------------------------------------------------------------------------------------------

void SummonMenu::initButtons()
{

    int buttonWidth = rc.w/3;
    int buttonHeight = guiUnit*0.75;

    const char* captions[]={"Cancel","Done"};

    for (int i=0;i<2;i++)
        controlButtons[i]= new SimpleButton(texButtons,rc.x + buttonWidth/4 +(rc.w/2)*(i&1),
                                     rc.y + rc.h-guiUnit,
                                     buttonWidth,buttonHeight,
                                     captions[i],
                                     BT_PUSHONLY,GLASS_BUTTON_ALPHA  );


    buttonWidth = 2*guiUnit;
    buttonHeight = 2*guiUnit;
    int offsetx= (rc.w- 2*buttonWidth)/4;

    for (int i=0;i<NUM_BALL_BUTTONS;i++)
    {
        ballTexts[i][0]=0;


        int bx=rc.x + offsetx +(rc.w/2)*(i&1);
        int by=rc.y + rc.h-guiUnit*( 1 + 2.5*(1+ i/2));

        rcBalls[i]={ bx+guiUnit/2, by+guiUnit/2, guiUnit,guiUnit };

        int blocklevel=-1;;
        playerStats->isSummonAvailable(i+1,level,&blocklevel);// button 0 is unlockable 1 and so on

        bool active=createButtonMessage(i,blocklevel);

        if (active)
            itemsAvailable=true;

        ballButtons[i]= new SimpleButton( texButtons,bx,by,
                                     buttonWidth,buttonHeight,
                                     " ",//ballTexts[i],
                                     BT_PUSHBUTTON, SIMPLE_BUTTON_ALPHA  );
        if (!active)
        {
           ballButtons[i]->disable();
           ballAlpha[i]=ALPHA_DISABLED;
        }

        else
          if (selectedItem==i+1)
          {
                ballButtons[i]->setState(BUTTONSTATE_PUSHEDIN);
                ballAlpha[i]=ALPHA_SELECTED;
          }
          else
          {
               ballButtons[i]->setState(BUTTONSTATE_IDLE);
               ballAlpha[i]=ALPHA_AVAILABLE;
          }


    }

}

//------------------------------------------------------------------------------------------------------

void SummonMenu::loadImages()
{

    texButtons  = glbTexManager->getTexturePtr(glbTexManager->insertTexture("buttons/buttons_blue.png",SKIN_UNIVERSAL,true));

    for (int i=0;i<2;i++)
        texMessages[i]= new BaseTexture;

    for (int i=0;i<NUM_BALL_BUTTONS+1;i++)
        texBallMessages[i]= new BaseTexture;


    const char* names[] = {"balls/ball_bomb.png","balls/ball_shifter.png","balls/timestop.png","balls/ball_painter.png","balls/ball_leveler.png","balls/ball_chrome.png"};

    for (int i=0;i<NUM_BALL_BUTTONS;i++)
        texBalls[i]=glbTexManager->getTexturePtr( glbTexManager->insertTexture(names[i],SKIN_UNIVERSAL,true));

    texStatic=NULL;
}

//------------------------------------------------------------------------------------------------------

void SummonMenu::createMessages()
{


    texMessages[0]->fromText("Ball Summoning",{230,230,230},glbFont);
    if (itemsAvailable)
    {
         texMessages[1]->fromText("Choose a ball to appear next,\nor stop time for one turn",
                                 {220,220,220},glbMessageFont,rc.w-guiUnit);
    }
    else
    {
         texMessages[1]->fromText("There are no available items\nto be used in this level",
                                 {220,220,220},glbMessageFont,rc.w-guiUnit);
    }

    for (int i=0;i<NUM_BALL_BUTTONS;i++)
    {
        texBallMessages[i]->fromText(ballTexts[i],{255,255,255},glbMessageFont);
    }
    texBallMessages[NUM_BALL_BUTTONS]->fromText("Selected",{255,255,255},glbMessageFont);

}
//------------------------------------------------------------------------------------------------------

bool SummonMenu::isDone ( bool *commit, int *item)
{

    if (done)
    {
        *commit = commitChanges;
        *item = selectedItem;
    }

    return done;
}
//------------------------------------------------------------------------------------------------------

void SummonMenu::handleEvents(SDL_Event& event)
{

    int i;
    int prevSelected=selectedItem;

    for (i=0;i<NUM_CONTROL_BUTTONS;i++)
            if (controlButtons[i]->isEnabled())
                controlButtons[i]->handleEvents();

    for (i=0;i<NUM_BALL_BUTTONS;i++)
            if (ballButtons[i]->isEnabled())
                ballButtons[i]->handleEvents();


    for (i=0;i<NUM_CONTROL_BUTTONS;i++)
        if (controlButtons[i]->getState()==BUTTONSTATE_PUSHEDHOVER)
        {
            switch (i)
            {
                case ID_BTN_CANCEL: commitChanges=false; done=true; break;
                case ID_BTN_DONE: commitChanges=true;  done=true; break;
            }
        }

    bool bselected=false;

     for (int i=0;i<NUM_BALL_BUTTONS;i++)
     {
        if (ballButtons[i]->isEnabled())
        {
            if (ballButtons[i]->isPushedIn())
            {
                bselected=true;
                if (selectedItem != i+1)
                {
                    if (selectedItem!=UNL_NONE)
                    {
                        ballButtons[selectedItem-1]->setState(BUTTONSTATE_IDLE);
                        ballAlpha[selectedItem-1]=ALPHA_AVAILABLE;
                    }
                    selectedItem=i+1;
                    ballAlpha[ i ]=ALPHA_SELECTED;
                }
            }
            else ballAlpha[i]=ALPHA_AVAILABLE;
        }
     }

     if (!bselected)
        selectedItem=UNL_NONE;

    if (selectedItem!=prevSelected)
        invStatic=true;
}

//------------------------------------------------------------------------------------------------------

void SummonMenu::logic(float dt)
{
    if (  invStatic)
        updateStaticImage();

    for (int i=0;i<NUM_BALL_BUTTONS;i++)
    {
        if (ballEffects[i]!=NULL)
            ballEffects[i]->update(dt);
    }

}
//------------------------------------------------------------------------------------------------------
void SummonMenu::createEffects()
{

    FPoint pos;
    for (int i=0;i<NUM_BALL_BUTTONS;i++)
    {
        ballEffects[i]=NULL;
        pos=ballButtons[i]->getOrigin();
        pos+=FPoint(guiUnit,guiUnit);

        switch (i)
        {
            case ID_BTN_BOMB:          ballEffects[i] = new FuseEffect( pos, FPoint(27.,-32).GetNormal()*guiUnit*0.47,guiUnit/8,FPoint(0,2*guiBox->h) ); break;

            case ID_BTN_SHIFTER:  ballEffects[i]= new PixiesEffect( pos, COLOR_SHIFTER, guiUnit/8); break;

            case ID_BTN_PAINTER:       ballEffects[i]= new PixiesEffect( pos, COLOR_PAINTER, guiUnit/8); break;

            case ID_BTN_LEVELER:       ballEffects[i]= new PixiesEffect( pos, COLOR_LEVELER, guiUnit/8); break;

            case ID_BTN_CHROME:        ballEffects[i]= new PixiesEffect( pos, COLOR_CHROME, guiUnit/8); break;
        }
    }
}



//------------------------------------------------------------------------------------------------------
void SummonMenu::updateStaticImage()
{
    if (texStatic==NULL)
    {
        texStatic = new BaseTexture;

        if (!texStatic->createBlank(rc.w,rc.h,SDL_TEXTUREACCESS_TARGET) )
        {
    #ifdef CONSOLE_LOG
            sprintf(glbLogMsg, "InGame:texStatic creation Failed\n");
            mySDL_Log(glbLogMsg);
    #endif // CONSOLE_LOG

            delete texStatic;
            texStatic=NULL;
        }
        else
            texStatic->useAlphaBlending(true);
    }

    if (texStatic!=NULL)
    {
        texStatic->setAsRenderTarget();

        FPoint offset( -rc.x, -rc.y );

        renderContent(offset);

        texStatic->resetRenderTarget();
        invStatic=false;
    }
}

//------------------------------------------------------------------------------------------------------
void SummonMenu::renderMessages(const FPoint& offset)
{
    int px,py,i;



    for (int i=0;i<NUM_BALL_BUTTONS;i++)
    {
        int msg=(selectedItem==i+1)?NUM_BALL_BUTTONS:i;

        texBallMessages[msg]->render(offset.x+rcBalls[i].x+(guiUnit-texBallMessages[msg]->getWidth())/2,
                                offset.y+rcBalls[i].y+guiUnit*1.25,
                                NULL,-1,std::max(150,ballAlpha[i]));
    }

    px = offset.x + rc.x+guiUnit/2;
    py = offset.y+rcBalls[NUM_BALL_BUTTONS-1].y -guiUnit*1.75;

    for (i=1;i>=0;i--)
    {
        texMessages[i]->render(px,py);
        py-=guiUnit*0.75;
    }
}

//------------------------------------------------------------------------------------------------------
void SummonMenu::render(float phase)
{
    FPoint offset(0.f,0.f);

    if (texStatic!=NULL)
    {
        texStatic->render(rc.x,rc.y);
    }
    else renderContent(offset);

    for (int i=0;i<NUM_BALL_BUTTONS;i++)
    {
        if (ballEffects[i]!=NULL)
            ballEffects[i]->render( ballAlpha[i]/255.);
    }

    for (int i=0;i<NUM_CONTROL_BUTTONS;i++)
       controlButtons[i]->render(offset);

}
//------------------------------------------------------------------------------------------------------
void SummonMenu::renderContent(const FPoint& offset)
{
    SDL_Rect rcTarget=rc;

    rcTarget.x +=offset.x;
    rcTarget.y +=offset.y;

    SDL_SetRenderDrawColor(glbRenderer, 0,0,0,210);
    SDL_RenderFillRect(glbRenderer,&rcTarget);

    renderMessages(offset);

    for (int i=0;i<NUM_BALL_BUTTONS;i++)
    {
        if (texBalls[i]!=NULL)
            texBalls[i]->render(rcBalls[i].x+offset.x , rcBalls[i].y+offset.y,&(rcBalls[i]),-1,ballAlpha[i]);
    }
}




