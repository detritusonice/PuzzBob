#include "base.h"
#include "basefuncs.h"

#include "FPoint.h"
#include "texture.h"
#include "screenEffects.h"
#include "gameSettings.h"

//#include <cstdio>
#include <cstdlib>
//#include <cmath>
#include "myMath.h"

#include <string>
#include <cstdio>




AirBubbles::AirBubbles(SDL_Rect* rc)
{
    effectRect = (*rc);
    numBalls = NUM_BALLCOLORS-NUM_SPECIALBALLS;

    init();
}

AirBubbles::~AirBubbles()
{
}

void AirBubbles::init()
{

    initBallRects();
    loadImages();

    effectAlpha=160;

}

void AirBubbles::loadImages()
{
    const char* names[NUM_BALLCOLORS-NUM_SPECIALBALLS]={

                                                    "ball_green.png","ball_blue.png","ball_purple.png",
                                                    "ball_red.png","ball_orange.png","ball_yellow.png",
                                                    "balls/ball_painter.png","balls/ball_shifter.png"};

    for (int i=0;i<numBalls;i++)
    {
        if (i<6)//POTENTIAL PROBLEM HERE: make sure you correctly track the images above...
            handles[i]=glbTexManager->insertTexture(names[i],gameSettings->skin,true);
        else
            handles[i]=glbTexManager->insertTexture(names[i],SKIN_UNIVERSAL,true);
    }
}

void AirBubbles::initBallRects()
{
    dist_extra= ((!gameSettings->fullSceneComplexity)?7:1);

    int unit = std::min(effectRect.w,effectRect.h) / 10;


    float dist=NUM_BALLCOLORS+dist_extra;
    float step=(dist-1)/numBalls;

    for (int i=0;i<numBalls;i++)
    {

        ballRect[i].w=6.*unit/dist;
        ballRect[i].h=6.*unit/dist;
        ballRect[i].x=0;
        ballRect[i].y=0;

        v[i]=FPoint(0.,-effectRect.h/dist);

        p[i].x=effectRect.w/10 + rand()%((int)(effectRect.w*0.8));
        p[i].y=effectRect.y/10 + rand()%((int)(effectRect.h*0.8));

        dist-=step;
    }
}
//-----------------------------------------------
void AirBubbles::moveEffect(const FPoint &mousePos)
{

    float cx=effectRect.w/2.;

    float dist=NUM_BALLCOLORS+dist_extra;
    float step=(dist-1)/numBalls;


    for (int i=0;i<numBalls;i++)
    {
          v[i].x=(mousePos.x-cx)/dist;

          dist-=step;
    }

}
//-----------------------------------------------
void AirBubbles::changeEffectAlpha(int diff)
{
    effectAlpha =std::max( 0, std::min( 255, effectAlpha+diff ) );
}
//-----------------------------------------------
bool AirBubbles::update(float dt, float phase)
{

    float dist=NUM_BALLCOLORS+dist_extra;
    float step=(dist-1)/numBalls;

    float wind = 0.5*effectRect.w*mySine(phase/7.);

    for (int i=0;i<numBalls;i++)
    {

          p[i].y += dt*v[i].y;
          p[i].x += dt*(v[i].x+wind/dist);

          if (p[i].y < -ballRect[i].h)
          {
                p[i].y=effectRect.h + ballRect[i].h;
                p[i].x=effectRect.x/10. + rand()%(int)(effectRect.w*0.8)-v[i].x ;
          }
          dist-=step;
    }
    return true;
}
//----------------------------------------------------
void AirBubbles::render(float alphaModifier)
{

    PROFILER_START();

    for (int i=0;i<numBalls;i++)
    {
        if ( !gameSettings->fullSceneComplexity &&(ballRect[i].w < guiUnit/2)) continue;
        if (p[i].y > effectRect.h) continue;
        if (p[i].x < -ballRect[i].w) continue;
        if (p[i].x > effectRect.w)  continue;

        (glbTexManager->getTexturePtr(handles[i]))->render( p[i].x, p[i].y,
                                                           &(ballRect[i]),
                                                           -1,
                                                           effectAlpha*alphaModifier
                                                            );
    }
    PROFILER_STOP();

}
//-----------------------------------------------------
void AirBubbles::resize(SDL_Rect* rc)
{
    effectRect = (*rc);
    initBallRects();
}
//==========================FLASH============================================

Flash::Flash( float effectDuration,SDL_Rect* rc)
{
    shine(effectDuration,rc);
    timeLeft=0.f;//don't want to shine upon construction
    init();
}

//-----------------------------------------------

Flash::~Flash()
{

}
//------------------------------------------------

void Flash::shine(float effectDuration, SDL_Rect* rc,const FPoint& position, float rad)
{
    if ((gameSettings->flashOnExplosions) || (rad>0.) )//used for leveler explosion
    {
        duration= effectDuration;
        timeLeft = effectDuration;
        pos=position;
        height=rad;
        effectRect = (*rc);
    }
}
//-----------------------------------------------
void Flash::loadImages()
{
    handle=glbTexManager->insertTexture("effects/nova_flash.png",SKIN_UNIVERSAL,true);//,true);
}
//--------------------------------------------------

void Flash::init()
{
    loadImages();

    alphaValue=255;
    effectAlpha=255;
}

//-----------------------------------------------

void Flash::resize(SDL_Rect *rc)
{
    effectRect = (*rc);
}

//-----------------------------------------------

bool Flash::update(float dt, float phase)
{

    if (timeLeft>0)
    {
        alphaValue=127.*timeLeft/duration;
        timeLeft-=dt;
        return true;
    }
    return false;
}

//-----------------------------------------------

void Flash::render(float alphaModifier)
{
    PROFILER_START();
    if (timeLeft>0)
    {
        float a=alphaValue*alphaModifier*effectAlpha/255.;

         if (gameSettings->flashOnExplosions)
             glbTexManager->getTexturePtr(handle)->render(
                                                        effectRect.x,
                                                        effectRect.y,
                                                        &effectRect,
                                                        -1,
                                                        a);
        if (height>0.)
        {
            float t=(timeLeft/duration);
           // t*=t;
            //t*=t;
            float r=(1.-t)*height;
            SDL_Rect rc={effectRect.x,pos.y-r,effectRect.w,2*r};
            glbTexManager->getTexturePtr(handle)->render(
                                                            rc.x,
                                                            rc.y,
                                                            &rc,
                                                            -1,
                                                            2*a*t);

            rc={effectRect.x,pos.y-r/16 -1,effectRect.w,r/8 +2};
            glbTexManager->getTexturePtr(handle)->render(
                                                            rc.x,
                                                            rc.y,
                                                            &rc,
                                                            -1,
                                                            2*a);

        }
    }
    PROFILER_STOP();
}

//-----------------------------------------------

void Flash::changeEffectAlpha(int diff)
{
    effectAlpha =std::max( 0, std::min( 255, effectAlpha+diff ) );
}

//-----------------------------------------------

void Flash::moveEffect(const FPoint &newPos)
{
    pos = newPos;
}

//===============================NOVAFLASH====================================

NovaFlash::NovaFlash( FPoint position, float effectDuration,SDL_Rect* rc)
{
    shine(position,effectDuration,rc);
    timeLeft=0.f;//don't want to shine upon construction
    init();
}

//-----------------------------------------------

NovaFlash::~NovaFlash()
{

}
//------------------------------------------------

void NovaFlash::shine(FPoint position, float effectDuration, SDL_Rect* rc)
{

    pos=position;
    duration= effectDuration;
    timeLeft = effectDuration;
    effectRect = (*rc);
}
//-----------------------------------------------
void NovaFlash::loadImages()
{
    handles[HANDLE_FLASH]=glbTexManager->insertTexture("effects/nova_flash.png",SKIN_UNIVERSAL,true);
    handles[HANDLE_NOVA]=glbTexManager->insertTexture("effects/nova_white.png",SKIN_UNIVERSAL,true);
}
//--------------------------------------------------

void NovaFlash::init()
{
    loadImages();

    initRects();

    pos = FPoint( effectRect.w/2, effectRect.h/2);

    for (int i=0;i<NUM_HANDLES;i++)
    {
        angles[i]=0.;
        alphaValues[i]=255;
    }
    effectAlpha=255;
}

//-----------------------------------------------

void NovaFlash::initRects()
{

    rects[HANDLE_FLASH] = effectRect;

    rects[HANDLE_NOVA].x=rects[HANDLE_NOVA].y;

    rects[HANDLE_NOVA].w= std::min(effectRect.w,effectRect.h)/4;
    rects[HANDLE_NOVA].h=rects[HANDLE_NOVA].w;
}

//-----------------------------------------------

void NovaFlash::resize(SDL_Rect *rc)
{
    effectRect = (*rc);
    initRects();
}

//-----------------------------------------------

bool NovaFlash::update(float dt, float phase)
{

    if (timeLeft>0)
    {
        float t=timeLeft*1./duration;
        float refSize=std::min(effectRect.w,effectRect.h)/4;

        if (t>0.999) rects[HANDLE_NOVA].w = refSize/10 ;
        else
            rects[HANDLE_NOVA].w= refSize*mySine(1-t)*(t/(1-t));

        rects[HANDLE_NOVA].h=rects[HANDLE_NOVA].w;

        angles[HANDLE_NOVA] = 180.*(3*phase)/PI;

        alphaValues[HANDLE_FLASH] =127*t;

        alphaValues[HANDLE_NOVA] =255*t;

        timeLeft-=dt;
        return true;
    }
    return false;
}

//-----------------------------------------------

void NovaFlash::render(float alphaModifier)
{
    if (timeLeft>0)
    {
        (glbTexManager->getTexturePtr(handles[HANDLE_FLASH]))->render(
                                                                effectRect.x,
                                                                effectRect.y,
                                                                &(rects[HANDLE_FLASH]),
                                                                -1,
                                                                (effectAlpha/255.)*
                                                                alphaValues[HANDLE_FLASH]
                                                                *alphaModifier );

        (glbTexManager->getTexturePtr(handles[HANDLE_NOVA]))->render(
                                                                pos.x - rects[HANDLE_NOVA].w/2,
                                                                pos.y - rects[HANDLE_NOVA].h/2,
                                                                &(rects[HANDLE_NOVA]),
                                                                -1,
                                                                (effectAlpha/255.)*
                                                                 alphaValues[HANDLE_NOVA]
                                                                 *alphaModifier,
                                                                 angles[HANDLE_NOVA] );
    }
}

//-----------------------------------------------

void NovaFlash::changeEffectAlpha(int diff)
{
    effectAlpha =std::max( 0, std::min( 255, effectAlpha+diff ) );
}

//-----------------------------------------------

void NovaFlash::moveEffect(const FPoint &newPos)
{
    pos = newPos;
}
//==============================================================================================

TrailsEffect::TrailsEffect(int w, int h, const SDL_Color& clr ,int len)
{
    width=w;
    height=h;
    length=len;
    bgColor = clr;//{c.r,cg,b};

    buf[0]=buf[1]=NULL;

    current = 0;
}

TrailsEffect::~TrailsEffect()
{
    for (int i=0;i<2;i++)
        if (buf[i]!=NULL)
        {
            delete buf[i];
            buf[i]=NULL;
        }
}


void TrailsEffect::initFrame(  )
{
    initFrame( bgColor );
}

void TrailsEffect::initFrame( const SDL_Color& clr )
{
    PROFILER_START();

   if (buf[current]==NULL)
    {
        buf[current]= new BaseTexture;
        if (!buf[current]->createBlank(width,height,SDL_TEXTUREACCESS_TARGET))
        {

#ifdef CONSOLE_LOG
            sprintf(glbLogMsg, "TrailsEffect buffer textures creation failed\n");
            mySDL_Log(glbLogMsg);
#endif // CONSOLE_LOG
            delete buf[current];
            buf[current]=NULL;
            PROFILER_STOP();
            return;
        }

        buf[current]->useAlphaBlending(true);
    }


    buf[current]->setAsRenderTarget( clr.r,clr.g, clr.b );

    PROFILER_STOP();
}

void TrailsEffect::renderPreviousFrame()
{
    PROFILER_START();
    if (buf[current^1]!=NULL)
        buf[current^1]->render(0,0,NULL,-1,230);//210);for rendering on actual background
    PROFILER_STOP();
}

void TrailsEffect::renderFrame(int x, int y)
{
    PROFILER_START();
   if (buf[current]!=NULL)
   {
        buf[current]->resetRenderTarget();
        buf[current]->render(x,y,NULL,-1,255);
        current= current^1;
   }
   PROFILER_STOP();
}

void TrailsEffect::includeInEffect()
{
   if (buf[current]!=NULL)
        buf[current]->setAsRenderTargetNoFlush();
}

void TrailsEffect::excludeFromEffect()
{
    if (buf[current]!=NULL)
        buf[current]->resetRenderTarget();
}

