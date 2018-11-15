#include "base.h"
#include "basefuncs.h"
#include "sliderBar.h"
#include "texture.h"
#include "myMath.h"

#include <cstdio>

/*
enum SLIDERSTATES
{
    SLIDERSTATE_IDLE=0,
    SLIDERSTATE_DISABLED,
    SLIDERSTATE_PRESSED,
    NUM_SLIDERSTATES
};
*/

BaseSlider::BaseSlider(int x, int y,int w, int h, int scrollerDirection, float newLevel,SDL_Color fgClr, SDL_Color bgClr)
{
    #ifdef CONSOLE_LOG
        sprintf(glbLogMsg,"BaseSlider: constructor:\n");
        mySDL_Log(glbLogMsg);
    #endif // CONSOLE_LOG

    dFgColor = fgClr;
    dBgColor= bgClr;

    init();
    loadImages();


    setDirection(scrollerDirection);
    setPosition(x,y,w,h);

    computeRects();

    setLevel(newLevel);
}
//-------------------------------------------------------------------
BaseSlider::~BaseSlider()
{
}


void BaseSlider::loadImages()
{
        texBackground = glbTexManager->getTexturePtr(glbTexManager->insertTexture("volumeBarBG.png",SKIN_UNIVERSAL,true));
        texForeground = glbTexManager->getTexturePtr(glbTexManager->insertTexture("volumeBarFG.png",SKIN_UNIVERSAL,true));
        texActive = glbTexManager->getTexturePtr(glbTexManager->insertTexture("volumeBarActive.png",SKIN_UNIVERSAL,true));
        texHandle = glbTexManager->getTexturePtr(glbTexManager->insertTexture("volumeBarHandle.png",SKIN_UNIVERSAL,true));
}
//-------------------------------------------------------------------

void BaseSlider::init()
{
    setState(SLIDERSTATE_IDLE);

    setOnScrollPage(false);

    prevMouseState=0;
    clickOutside=false;

    touchPoint = {0,0};
    wasModified = false;
}
//-------------------------------------------------------------------
void BaseSlider::setOnScrollPage(bool onScrollPage)
{
        isOnScrollPage = onScrollPage;
}

//-------------------------------------------------------------------
void  BaseSlider::setOrigin(int x, int y)
{
    origin.x=x;
    origin.y=y;
}
//-------------------------------------------------------------------
FPoint  BaseSlider::getOrigin()
{
    return origin;
}
//-------------------------------------------------------------------
bool BaseSlider::levelChanged()
{
    return wasModified;
}
//-------------------------------------------------------------------
bool BaseSlider::handleEvents(FPoint parentPosition)
{
    wasModified=false;

    if ( state == SLIDERSTATE_DISABLED ) return false;

    int mx,my;
    bool eventHandled=false;

    rcBox.x = parentPosition.x + origin.x;
    rcBox.y = parentPosition.y + origin.y;


    Uint8 mouseBtn=SDL_GetMouseState(&mx,&my);

    if ( ( rcBox.x<=mx ) &&   (mx<=rcBox.x+rcBox.w )&&  (rcBox.y<=my ) && (my<=rcBox.y+rcBox.h ) )//hovering
    {

            if ( mouseBtn&SDL_BUTTON(1) &&(clickOutside==false))//leftclick
            {
                if (!prevMouseState&SDL_BUTTON(1))
                {
                    touchPoint.x=mx;
                    touchPoint.y=my;
                }

                state=SLIDERSTATE_PRESSED;
            }
            else
            {
                if ( (prevMouseState&SDL_BUTTON(1))&&(clickOutside==false) )//was pressed , now released inside the button
                    state=SLIDERSTATE_IDLE;
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

            state=SLIDERSTATE_IDLE;

        }
        else
        {//is now pressed and was not pressed=clicked outside button
            if(!(prevMouseState&SDL_BUTTON(1)))
                clickOutside=true;
            else //this buttons has a pending press-in, do not allow scrolling gesture
                if (!clickOutside)
                    state=SLIDERSTATE_PRESSED;
        }
    }

    if (state==SLIDERSTATE_PRESSED)
    {
        float newLevel;

        if (direction==VERTICAL)
            newLevel=(rcBox.y+rcForeground.y+rcForeground.h-my)/(rcForeground.h*1.f);
        else
            newLevel=(mx-(rcBox.x+rcForeground.x))/(rcForeground.w*1.f);

        newLevel=std::max(0.f,std::min(1.f,newLevel));

        if (newLevel!=level)
        {
            setLevel(newLevel);
            wasModified=true;
        }
        eventHandled=true;
    }
#ifdef CONSOLE_LOG
    if (eventHandled)
    {
        sprintf(glbLogMsg,"BaseSlider HandleEvent end\n");
        mySDL_Log(glbLogMsg);
    }
#endif
    return eventHandled;
}
//-------------------------------------------------------------------

int BaseSlider::getState()
{
    return state;
}

//-------------------------------------------------------------------

int BaseSlider::setState(int newState)
{
    if ( (newState>=0) && (newState<NUM_SLIDERSTATES) )
    {
        int oldState=state;
        state=newState;

        return oldState;
    }
    return -1;
}

//-------------------------------------------------------------------

void BaseSlider::enable()
{
    state=SLIDERSTATE_IDLE;
    prevMouseState=0;
}

//-------------------------------------------------------------------

void BaseSlider::disable()
{
    setState(SLIDERSTATE_DISABLED);
    prevMouseState=0;
}

//-------------------------------------------------------------------

bool BaseSlider::isEnabled()
{
    return (state!=SLIDERSTATE_DISABLED);
}

//--------------------------------------------------------------------

void BaseSlider::setPosition(int x,int y, int w, int h)
{
    rcBox.x=x;
    rcBox.y=y;
    if (w!=0)
        rcBox.w=w;
    if (h!=0)
        rcBox.h=h;
    if ((w!=0) && (h!=0))
    {
        setOrigin(x,y);
    }
}
//-------------------------------------------------------------------
void BaseSlider::setDirection(int newDirection)
{
    //if ((newDirection==VERTICAL) || (newDirection==HORIZONTAL) )
    direction=newDirection;
}

//-------------------------------------------------------------------
void BaseSlider::setLevel(float newLevel)
{
    if ((newLevel>=0.f) && (newLevel<=1.f))
    {
        level=newLevel;
        computeActive();
    }
}

//-------------------------------------------------------------------

float BaseSlider::getLevel()
{
    return level;
}

//---------------------------------------------------------------------
void BaseSlider::computeRects()
{
    int lb =(int) guiUnit/2;//line breadth

    lb/=8;
    if (lb==0) lb++;
    lb*=8;

    if (direction==VERTICAL)
        rcBackground={(rcBox.w -lb)/2,lb/2,lb,rcBox.h-lb};
    else
        rcBackground={lb/2,(rcBox.h-lb)/2,rcBox.w-lb,lb};

    rcHandle = {0,0,(int)(1.5*lb),(int)(1.5*lb)};

    lb=(int)lb/2.;

    rcForeground={rcBackground.x+lb/2, rcBackground.y+lb/2,rcBackground.w-lb,rcBackground.h-lb};


}
//-------------------------------------------------------------------
void BaseSlider::computeActive()
{
    int border=1;
    if (direction==HORIZONTAL)
    {
        rcActive= {rcForeground.x+border, rcForeground.y+border, (int)((rcForeground.w-2*border)*level),rcForeground.h-2*border};
        rcHandle.x=rcActive.x+rcActive.w - rcHandle.w/2;
        rcHandle.y=rcActive.y+ (rcActive.h - rcHandle.h)/2;

    }
    else
    {
        int length=(rcForeground.h-2*border)*level;
        rcActive= {rcForeground.x+border, rcForeground.y+rcForeground.h-border-length,rcForeground.w-2*border,length};

        rcHandle.x=rcActive.x+(rcActive.w - rcHandle.w)/2;
        rcHandle.y=rcActive.y - rcHandle.h/2;

    }

}
//-------------------------------------------------------------------
void BaseSlider::render(const FPoint& offset, float alphaModifier)
{

    FPoint pos(offset.x+origin.x,offset.y+origin.y);

    if (!isEnabled())
    {
        SDL_Rect rc={(int)pos.x,(int)pos.y,rcBox.w,rcBox.h};

        SDL_SetRenderDrawColor(glbRenderer,dFgColor.r,dFgColor.g,dFgColor.b,255*alphaModifier);
        SDL_RenderDrawRect(glbRenderer,&rc);

        rc=rcBackground;

        rc.x+=pos.x;
        rc.y+=pos.y;

        SDL_SetRenderDrawColor(glbRenderer,dBgColor.r,dBgColor.g,dBgColor.b,255*alphaModifier);
        SDL_RenderFillRect(glbRenderer,&rc);

        rc=rcForeground;

        rc.x+=pos.x;
        rc.y+=pos.y;

        SDL_SetRenderDrawColor(glbRenderer,dFgColor.r,dFgColor.g,dFgColor.b,255*alphaModifier);
        SDL_RenderFillRect(glbRenderer,&rc);

        rc=rcActive;

        rc.x+=pos.x;
        rc.y+=pos.y;

        SDL_SetRenderDrawColor(glbRenderer,255,255,255,255*alphaModifier);
        SDL_RenderFillRect(glbRenderer,&rc);
    }
    else
    {
        SDL_Rect rc={(int)pos.x,(int)pos.y,rcBox.w,rcBox.h};

        SDL_SetRenderDrawColor(glbRenderer,dFgColor.r,dFgColor.g,dFgColor.b,255*alphaModifier);
        SDL_RenderDrawRect(glbRenderer,&rc);

        texBackground->render(pos.x+rcBackground.x, pos.y+rcBackground.y,&rcBackground,-1,
                              255*alphaModifier);

        texForeground->render(pos.x+rcForeground.x, pos.y+rcForeground.y,&rcForeground,-1,
                              155*alphaModifier);

        texActive->render(pos.x+rcActive.x, pos.y+rcActive.y,&rcActive,-1,
                              220*alphaModifier,0,NULL,SDL_FLIP_HORIZONTAL);

        texHandle->render(pos.x+rcHandle.x, pos.y+rcHandle.y,&rcHandle,-1,
                              255*alphaModifier, (direction==VERTICAL?0:90));
    }

}

