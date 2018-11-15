#include "base.h"
#include "basefuncs.h"
#include "button.h"
#include "texture.h"
#include "inGame.h"
#include "gameSettings.h"

#include <cstdio>


SDL_Color buttonTextColor  = {0,0,0};


/*

enum BUTTONSTATES
{
    BUTTONSTATE_IDLE=0,
    BUTTONSTATE_HOVER,
    BUTTONSTATE_DISABLED,
    BUTTONSTATE_PUSHEDHOVER,
    BUTTONSTATE_PUSHEDIN,
    BUTTONSTATE_PRESSED,
    NUM_BUTTONSTATES
};

enum BUTTON_TYPES
{
    BT_PRESSONLY=0, //pressin
    BT_PUSHONLY,    //radiobutton
    BT_PUSHBUTTON   //push on-off
};

//-----------------------------------------------
//|             |               |               |
//|    IDLE     |     HOVER     |   DISABLED    |
//|             |               |               |
//-----------------------------------------------
//|             |               |               |
//| PUSHEDHOVER |   PUSHEDIN    |    PRESSED    |
//|             |               |               |
//-----------------------------------------------

*/

//-------------------------------------------------------------------


//-------------------------------------------------------------------

void BaseButton::init(bool isScrollButton)
{
    setState(BUTTONSTATE_IDLE);
    stateChange=false;
    prevMouseState=0;
    clickOutside=false;
    touchPoint={0,0};
    wasPressed=false;
    stateAlphaMod=1.;

    isScrollPageButton= isScrollButton;
}
//-------------------------------------------------------------------
void BaseButton::setOnScrollPage(bool isScrollButton)
{
        isScrollPageButton= isScrollButton;
}

//-------------------------------------------------------------------
void  BaseButton::setOrigin(int x, int y)
{
    origin.x=x;
    origin.y=y;
}
//-------------------------------------------------------------------
FPoint  BaseButton::getOrigin()
{
    return origin;
}
//-------------------------------------------------------------------
bool BaseButton::gotPressed()
{
    return wasPressed;
}
//-------------------------------------------------------------------
bool BaseButton::handleEvents(FPoint parentPosition)
{
    wasPressed=false;
    stateChange=false;

    if ( iButtonState==BUTTONSTATE_DISABLED ) return false;

    int mx,my;
    bool eventHandled=false;

    rcBox.x=parentPosition.x+origin.x;
    rcBox.y=parentPosition.y+origin.y;


    Uint8 mouseBtn=SDL_GetMouseState(&mx,&my);

    if ( ( rcBox.x<=mx ) &&   (mx<=rcBox.x+rcBox.w )&&  (rcBox.y<=my ) && (my<=rcBox.y+rcBox.h ) )//hovering
    {


            if ( mouseBtn&SDL_BUTTON(1) &&(clickOutside==false))//leftclick
            {
                if (!prevMouseState&SDL_BUTTON(1))
                {
                    touchPoint.x=mx;
                    touchPoint.y=my;
#ifndef PLATFORM_ANDROID
                    if (iButtonType==BT_PRESSONLY)
                    {
                        wasPressed=true;
                        playSound(SFX_TAP2,0);
                    }
#endif

                }
                FPoint dp(mx-touchPoint.x,my-touchPoint.y);
                if ((isScrollPageButton) &&(dp.GetLength()>guiUnit/2.) )
                {
                    if ((iButtonType!=BT_PRESSONLY) && pushedIn)
                        setState(BUTTONSTATE_PUSHEDHOVER);//is pushed and hovering
                    else
                        setState(BUTTONSTATE_HOVER);
                    eventHandled=false;

                    //preventing activation upon re-entry
                    clickOutside=true;
                    touchPoint.x=-100;
                    touchPoint.y=-100;

                }
                else
                {
                    setState(BUTTONSTATE_PRESSED);
                    eventHandled=true;
                }
            }
            else
            {
                if ((prevMouseState&SDL_BUTTON(1))&&(clickOutside==false) )//was pressed , now released inside the button
                {

                    if ( iButtonType != BT_PRESSONLY)
                    {
                        if ((iButtonType == BT_PUSHONLY)&&(pushedIn == true))
                        {
                        }//cannot be unpushed
                        else
                        {
                            pushedIn=!pushedIn;
                            eventHandled=true;
                            playSound(SFX_TAP1,0);
                        }
                    }
                    else
                    {
#ifdef PLATFORM_ANDROID
                        wasPressed=true;
                        playSound(SFX_TAP2,0);
#endif
                    }
                }

                if ((iButtonType!=BT_PRESSONLY) && pushedIn)
                    setState(BUTTONSTATE_PUSHEDHOVER);//is pushed and hovering
                else
                    setState(BUTTONSTATE_HOVER);
            }
            prevMouseState=mouseBtn;
            if (!mouseBtn&SDL_BUTTON(1))
                clickOutside=false;
    }
    else
    {
        if ( !(mouseBtn&SDL_BUTTON(1)) )//leftclick not kept pressed some time when outside
        {
            prevMouseState=0;//stopped being clicked
            clickOutside=false;

            if (iButtonState==BUTTONSTATE_PRESSED)
            {
                if ((iButtonType!=BT_PRESSONLY) && pushedIn )
                    setState(BUTTONSTATE_PUSHEDIN);
                else
                    setState(BUTTONSTATE_IDLE);
            }
        }
        else
        {//is now pressed and was not pressed=clicked outside button
            if(!(prevMouseState&SDL_BUTTON(1)))
                clickOutside=true;
            else //this buttons has a pending press-in, do not allow scrolling gesture
                if (!clickOutside)
                {
                    FPoint dp(mx-touchPoint.x,my-touchPoint.y);
                    if ((isScrollPageButton) &&(dp.GetLength()>guiUnit/2.) )
                    {
                        eventHandled=false;
                        //preventing activation upon re-entry
                        clickOutside=true;
                        touchPoint.x=-100;
                        touchPoint.y=-100;
                    }
                    else
                        eventHandled=true;
                }
        }
        //not hovering anymore
        if ((iButtonState==BUTTONSTATE_HOVER) || (iButtonState==BUTTONSTATE_PUSHEDHOVER))
        {
            if ((iButtonType!=BT_PRESSONLY) && pushedIn )
                setState(BUTTONSTATE_PUSHEDIN);
            else
                setState(BUTTONSTATE_IDLE);
        }
    }
#ifdef CONSOLE_LOG
    if (eventHandled)
    {
        sprintf(glbLogMsg,"Button HandleEvent TRUE\n");
        mySDL_Log(glbLogMsg);
    }
#endif
    return eventHandled;
}
//-------------------------------------------------------------------

int BaseButton::getState()
{
    return iButtonState;
}

//-------------------------------------------------------------------

int BaseButton::setState(int newState)
{
    if ( (newState>=0) && (newState<NUM_BUTTONSTATES) )
    {

        int oldState=iButtonState;
        iButtonState=newState;

        if (oldState!=newState)
            stateChange=true;
        else return oldState;

        if ( (newState!=BUTTONSTATE_PUSHEDHOVER)&&(newState!=BUTTONSTATE_PRESSED)&&(newState!=BUTTONSTATE_PUSHEDIN)&&(newState!=BUTTONSTATE_DISABLED))
        {//do not want to change state upon pressing. just when liftoff happens
            if ((iButtonType!=BT_PRESSONLY) && pushedIn )
            {
                pushedIn=false;//was pushed in but in the new state it sould not be
            }
        }
        else
         if ( (newState==BUTTONSTATE_PUSHEDHOVER) || (newState==BUTTONSTATE_PUSHEDIN))
        {
            if (iButtonType!=BT_PRESSONLY)
            {
                pushedIn=true;//must be pudhed in for these states
            }
        }

        if (newState==BUTTONSTATE_DISABLED)
                stateAlphaMod=0.5f;
        else
            stateAlphaMod=1.f;

        return oldState;
    }
    return -1;
}

//-------------------------------------------------------------------

void BaseButton::enable()
{
        if ((iButtonType!=BT_PRESSONLY) && pushedIn )
            setState(BUTTONSTATE_PUSHEDIN);
        else
            setState(BUTTONSTATE_IDLE);
        prevMouseState=0;
}

//-------------------------------------------------------------------

void BaseButton::disable()
{
    setState(BUTTONSTATE_DISABLED);
    prevMouseState=0;
}

//-------------------------------------------------------------------

bool BaseButton::isEnabled()
{
    return (iButtonState!=BUTTONSTATE_DISABLED);
}

//-------------------------------------------------------------------

bool BaseButton::isPushedIn()
{
        return pushedIn;
}
//--------------------------------------------------------------------

void BaseButton::setPosition(int x,int y, int w, int h)
{
    rcBox.x=x;
    rcBox.y=y;
    if (w!=0)
        rcBox.w=w;
    if (h!=0)
        rcBox.h=h;
    if ((w!=0) && (h!=0))
        setOrigin(x,y);
}
//--------------------------------------------------------------------
bool BaseButton::changedState()
{
    return stateChange;
}
//===================================================================================
void TextureButton::setCaption(const char* newCaption)
{
    if (newCaption!=NULL)
    {
        delete texCaption;
        texCaption=new BaseTexture;
        texCaption->fromText(newCaption,buttonTextColor,glbFont);
        texCaption->useAlphaBlending(texButton->usesAlphaBlending());
    }

}

//-------------------------------------------------------------------

void TextureButton::setClips()
{
    texButton->setEqualClipRects(2,NUM_BUTTONSTATES/2);
}

//===========================================================================

SimpleButton::SimpleButton(BaseTexture* texButtons,int x, int y, int w, int h,const char* strCaption,int buttonType, Uint8 alphaValue )
{
    #ifdef CONSOLE_LOG
        sprintf(glbLogMsg,"TextureButton: %s constructor:\n",strCaption);
        mySDL_Log(glbLogMsg);
    #endif // CONSOLE_LOG

    texButton   =   texButtons;//loaded outside (using texture manager?)

    texCaption = new BaseTexture();

    if (strCaption!=NULL)
        texCaption->fromText( strCaption,buttonTextColor,glbFont);
    else
        texCaption->fromText( "",buttonTextColor,glbFont);

    texCaption->useAlphaBlending(texButton->usesAlphaBlending());

    mAlphaValue=alphaValue;

    setClips();
    setPosition(x,y,w,h);

    texButton->addInstance();

    iButtonType=buttonType;
    pushedIn=false;
    init();
}

//-------------------------------------------------------------------

SimpleButton::~SimpleButton()
{
    delete texCaption;

    texButton->destroyInstance();

}

//-------------------------------------------------------------------
void SimpleButton::render(const FPoint& offset, float alphaModifier)
{
    texButton->render(origin.x+offset.x, origin.y+offset.y, &rcBox, iButtonState,mAlphaValue*alphaModifier*stateAlphaMod);

    SDL_Rect rcCaption={ 0,0, texCaption->getWidth(),texCaption->getHeight()};

    if (rcCaption.w > rcBox.w*0.9)
    {
        float r= 1.f*rcCaption.h/rcCaption.w;

        rcCaption.w= rcBox.w*0.9;
        rcCaption.h= rcCaption.w*r;
    }

    texCaption->render(origin.x+offset.x+(rcBox.w - rcCaption.w)/2,
                       origin.y+offset.y+(rcBox.h-rcCaption.h)/2,
                       &rcCaption,-1,mAlphaValue*alphaModifier*stateAlphaMod);

}

//===========================================================================

GlassButton::GlassButton(BaseTexture* backgroundTexture,int x, int y, int w, int h,const char* strCaption,int buttonType, Uint8 alphaValue )
{
    #ifdef CONSOLE_LOG
        sprintf(glbLogMsg,"GlassButton: %s constructor:\n",strCaption);
        mySDL_Log(glbLogMsg);
    #endif // CONSOLE_LOG
    texBackground = backgroundTexture;

   // texButton =  glassTexture;
    texButton   =   glbTexManager->getTexturePtr(
                        glbTexManager->insertTexture("buttons/buttons_glass.png",SKIN_UNIVERSAL,true));

    texCaption = new BaseTexture();

    if (strCaption!=NULL)
        texCaption->fromText( strCaption,buttonTextColor,glbFont);
    else
        texCaption->fromText( "",buttonTextColor,glbFont);

    texCaption->useAlphaBlending(texButton->usesAlphaBlending());

    mAlphaValue=alphaValue;

    setClips();
    setPosition(x,y,w,h);

    texButton->addInstance();
    if (texBackground!=NULL)
        texBackground->addInstance();

    iButtonType=buttonType;
    pushedIn=false;
    init();
}

//-------------------------------------------------------------------

GlassButton::~GlassButton()
{
    delete texCaption;

    if (texBackground!=NULL)
        texBackground->destroyInstance();

    texButton->destroyInstance();

}
//-------------------------------------------------------------------

void GlassButton::render(const FPoint& offset, float alphaModifier)
{
    if (texBackground!=NULL)
        texBackground->render(origin.x+offset.x, origin.y+offset.y, &rcBox, iButtonState,mAlphaValue*alphaModifier*stateAlphaMod);

    texButton->render(origin.x+offset.x, origin.y+offset.y, &rcBox, iButtonState,180*alphaModifier*stateAlphaMod);

    SDL_Rect rcCaption={ 0,0, texCaption->getWidth(),texCaption->getHeight()};

    if (rcCaption.w > rcBox.w*0.9)
    {
        float r= 1.f*rcCaption.h/rcCaption.w;

        rcCaption.w= rcBox.w*0.9;
        rcCaption.h= rcCaption.w*r;
    }

    texCaption->render(origin.x+offset.x+(rcBox.w - rcCaption.w)/2,
                       origin.y+offset.y+rcBox.h*0.9125-rcCaption.h,
                       &rcCaption,-1,mAlphaValue*alphaModifier*stateAlphaMod);
}
//==========================================================================================
LevelButton::LevelButton(BaseTexture* backgroundTexture,  int x,int y,int w, int h, const char*strCaption,int buttonType,Uint8 alphaValue)
{
    #ifdef CONSOLE_LOG
        sprintf(glbLogMsg,"GlassButton: %s constructor:\n",strCaption);
        mySDL_Log(glbLogMsg);
    #endif // CONSOLE_LOG

    level=0;

    //if (backgroundTexture!=NULL)
      //      texBackground = backgroundTexture;
    //else
    texBackground=NULL;

    invHandle=glbTexManager->insertTexture("noLevel.png",SKIN_UNIVERSAL,true);

    texPtr=NULL;//glbTexManager->getTexturePtr(invHandle);


    texButton   =   glbTexManager->getTexturePtr(
                        glbTexManager->insertTexture("buttons/buttons_glass.png",SKIN_UNIVERSAL,true));

    texCaption = new BaseTexture();

    if (strCaption!=NULL)
        texCaption->fromText( strCaption,buttonTextColor,glbFont);
    else
        texCaption->fromText( "",buttonTextColor,glbFont);

    texCaption->useAlphaBlending(texButton->usesAlphaBlending());

    mAlphaValue=alphaValue;


    texStars[GM_TIMELIMIT-1]= glbTexManager->getTexturePtr(glbTexManager->insertTexture("star_cyan.png",SKIN_UNIVERSAL,true));
    texStars[GM_SHOTLIMIT-1]= glbTexManager->getTexturePtr(glbTexManager->insertTexture("star_green.png",SKIN_UNIVERSAL,true));
    texStars[GM_SCORELIMIT-1]= glbTexManager->getTexturePtr(glbTexManager->insertTexture("star_yellow.png",SKIN_UNIVERSAL,true));


    resetBeatenStats();

    setClips();
    setPosition(x,y,w,h);

    texButton->addInstance();


    iButtonType=buttonType;
    pushedIn=false;
    init();
}
//-------------------------------------------------------------------
LevelButton::~LevelButton()
{
    if (texPtr!=texBackground)
        texPtr->destroyInstance();
    texPtr=NULL;

    for (int i=0;i<3;i++)
        texStars[i]=NULL;

    freeBackground();

    delete texCaption;//texCaption->destroyInstance();

    texButton->destroyInstance();
}
//-------------------------------------------------------------------
void LevelButton::render(const FPoint& offset,float alphaModifier)
{
   if (texPtr!=NULL)
        texPtr->render(origin.x+offset.x, origin.y+offset.y, &rcBox, iButtonState,mAlphaValue*alphaModifier*stateAlphaMod);

    texButton->render(origin.x+offset.x, origin.y+offset.y, &rcBox, iButtonState,180*alphaModifier*stateAlphaMod);

    SDL_Rect rcCaption={ 0,0, texCaption->getWidth(),texCaption->getHeight()};

    if (rcCaption.w > rcBox.w*0.9)
    {
        float r= 1.f*rcCaption.h/rcCaption.w;
        rcCaption.w= rcBox.w*0.9;
        rcCaption.h= rcCaption.w*r;
    }

    texCaption->render(origin.x+offset.x+(rcBox.w - rcCaption.w)/2,
                       origin.y+offset.y+rcBox.h*0.9125-rcCaption.h,
                       &rcCaption,-1,mAlphaValue*alphaModifier*stateAlphaMod);

}
//----------------------------------------------------------------------
void LevelButton::freeBackground()
{
    if (texBackground!=NULL)
    {
        delete texBackground;
        texBackground=NULL;
    }
}
//-------------------------------------------------------------------

//-------------------------------------------------------------------
void LevelButton::setFile(const char* fname,int newlevel,bool changeCaption)
{
#ifdef CONSOLE_LOG
        sprintf(glbLogMsg,"LevelButton: setFile: [%s]\n",fname);
        mySDL_Log(glbLogMsg);
#endif

    char buffer[MAX_WELL_WIDTH*MAX_WELL_HEIGHT+1];

    if (theGame->loadRawLevel(fname,buffer,true,newlevel))
        createBackground(buffer);

    if (texBackground == NULL)
    {
        level=-1;
        texPtr=glbTexManager->getTexturePtr(invHandle);
        setState(BUTTONSTATE_DISABLED);
    }
    else
    {
        texPtr= texBackground;
        if (changeCaption)
            texCaption->fromText(fname,buttonTextColor,glbFont);
        setState(BUTTONSTATE_IDLE);
    }
}
//-------------------------------------------------------------------
void LevelButton::setLevel(int newLevel,bool changeCaption)
{
#ifdef CONSOLE_LOG
        sprintf(glbLogMsg,"LevelButton: setLevel: [%d]\n",newLevel);
        mySDL_Log(glbLogMsg);
#endif

    //freeBackground();

    if (newLevel<0)
    {
        texPtr=glbTexManager->getTexturePtr(invHandle);
        texCaption->fromText("----",buttonTextColor,glbFont);
        setState(BUTTONSTATE_DISABLED);
        level=-1;
        return;
    }
    resetBeatenStats();

    level = newLevel;

    if ( (gameSettings->showBeatenStats) &&(changeCaption))//browser level button
        setBeatenStats();


    char fname[70];
    sprintf(fname,"levels/width%02d/level%04d.bin",gameSettings->getWellWidthValue(),level);

    setFile(fname,level);

    if (level==-1)
        resetBeatenStats();

    if (changeCaption)
    {
        sprintf(fname,"%d",level+1);
        texCaption->fromText(fname,buttonTextColor,glbFont);
    }

}
//-------------------------------------------------------------------
void LevelButton::setBeatenStats()
{
    numStars=0;

    for (int i= GM_TIMELIMIT;i<=GM_SCORELIMIT;i++)
        if (playerStats->hasBeatenModeStat(level,i) )
        {
            showStar[i-1]=true;
            numStars++;
        }
    if (numStars==0)
        return;

    float step,offset;

    step=rcBox.w/5;
    offset=(3.5*rcBox.w/4 -(numStars-0.5)*step)/2;

    int k=0;
    for (int i=0;i<3;i++)
        if (showStar[i])
        {
            rcStars[i]= {offset , rcBox.h*0.70, step,step};//0.75,/9
            k++;
            offset+=step;
        }


}
//-------------------------------------------------------------------

void LevelButton::resetBeatenStats()
{
    for (int i=0;i<3;i++)
    {
        showStar[i]=false;
    }
    numStars=0;
}

//-------------------------------------------------------------------
void LevelButton::createBackground(char* buffer)
{
    bool success=true;

    if (texBackground!=NULL)
        if ( (texBackground->getHeight()!=rcBox.h) ||
             (texBackground->getWidth()!=rcBox.w) )
            {
                freeBackground();
            }


    if (texBackground==NULL)
    {
        texBackground = new BaseTexture;
        success = texBackground->createBlank( rcBox.w,rcBox.h,SDL_TEXTUREACCESS_TARGET);
    }


    if ( success )
    {
        texBackground->useAlphaBlending(true);
        texBackground->setAsRenderTarget();
        theGame->renderRawLevel(buffer);

    for (int i=0;i<3;i++)
        if (showStar[i])
            texStars[i]->render(rcStars[i].x,rcStars[i].y,&rcStars[i],-1,75,21*i);

        texBackground->resetRenderTarget();
    }
    else
        freeBackground();

}
//-------------------------------------------------------------------
int LevelButton::getLevel()
{
#ifdef CONSOLE_LOG
        sprintf(glbLogMsg,"LevelButton: getLevel: [%d]\n",level);
        mySDL_Log(glbLogMsg);
#endif
    return level;
}

InGame* LevelButton::theGame=NULL;

//----------------------------------------------------------------------
void LevelButton::setGameObject(InGame* gameObj)// using the already existent InGame state object's renderer and level loader to avoid replication
{
    theGame = gameObj;
}

