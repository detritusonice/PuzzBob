
#include "base.h"
#include "basefuncs.h"

#include "FPoint.h"
#include "texture.h"
#include "localEffects.h"
#include "gameSettings.h"

#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <string>

#include "myMath.h"// precomputed trig values for older cpus

//=============================================================================
void LocalEffect::start()
{
    timeElapsed = 0.;
    reset();
}
//----------------------------------------------------------
void LocalEffect::setAlpha(int newAlpha)
{
    alpha =std::max( 0, std::min( 255, newAlpha) );
}
//----------------------------------------------------------
void LocalEffect::deltaAlpha(int diff)
{
    alpha =std::max( 0, std::min( 255, alpha+diff) );
}
//----------------------------------------------------------
bool LocalEffect::hasExpired() const
{
    return ( timeElapsed >= duration );
}
//----------------------------------------------------------
void LocalEffect::setPosition(const FPoint &newPos)
{
    position = newPos;
}

//=============================================================================
void ScoreEffect::init()
{
    reset();
}
//----------------------------------------------------------
void ScoreEffect::loadImage()
{
    std::string filename="effects/score_";
    switch (value)
    {
        case SCORE_DROPPED      : filename+="dropped";break;
        case SCORE_EXPLODED     : filename+="exploded";break;
        case SCORE_MATCHED      : filename+="matched";break;
    }
    filename+=".png";

    imageHandle=glbTexManager->insertTexture( filename ,SKIN_UNIVERSAL,true);

}
//----------------------------------------------------------
void ScoreEffect::reset()
{
    timeElapsed=0.;
    alpha=255;
}
//----------------------------------------------------------
ScoreEffect::ScoreEffect (const FPoint& pos,  float maxSize, int val, float dur )
{
        position=pos;
        dimension=maxSize;
        duration=dur;
        value=val;

        loadImage();
        reset();

}
//----------------------------------------------------------
void ScoreEffect::resize( const SDL_Rect& rc)
{
    if ( hasExpired() ) return;

    dimension = std::min(rc.w,rc.h);
    setSize();
}
//-----------------------------------------------------
void ScoreEffect::setSize()
{

    float t= timeElapsed /duration;

    rc.w= dimension*(1+t)/2.;

    if (rc.w<0) rc.w=0;

    rc.h=rc.w;

}
//----------------------------------------------------------
bool ScoreEffect::update(float dt, float phase)
{
    timeElapsed+=dt;

    if ( hasExpired() ) return false;

    float t=timeElapsed/duration;

    setSize();
    setAlpha(255*(1-t*t));

    return true;
}
//----------------------------------------------------------
void ScoreEffect::render(float alphaModifier) const
{
    if ( hasExpired() ) return;

    (glbTexManager->getTexturePtr(imageHandle))->render(
                                                        position.x - rc.w/2,
                                                        position.y - rc.h/2,
                                                        &rc,
                                                        -1,
                                                        alpha*alphaModifier);
}

//=============================================================================
ExplosionFlashEffect::ExplosionFlashEffect(const FPoint& pos,float maxSize,int explosionColor,
                                            float dur,float rotSpeed)
{
    dimension=maxSize;
    duration=dur;
    rotationSpeed=rotSpeed;

    setPosition(pos);

    setColor(explosionColor);

    init();
}
//-----------------------------------------------------
void ExplosionFlashEffect::init()
{
    timeElapsed=duration+1;//Constructing expired. Triggering with start
    angle = 0;

    reset();
}

//-----------------------------------------------------
void ExplosionFlashEffect::reset()
{
    setSize();
    alpha=255;
}
//-----------------------------------------------------
void ExplosionFlashEffect::resize(const SDL_Rect &rcSize)
{
    dimension = std::min(rcSize.w,rcSize.h);
    setSize();
}
//-----------------------------------------------------
void ExplosionFlashEffect::setSize()
{

    float t= timeElapsed /duration;

    if (timeElapsed==0.)
        rc.w = dimension/10;
    else
        rc.w= dimension*mySine(t)*2.*(1-t)/t;

    if (rc.w<0) rc.w=0;

    rc.h=rc.w;

}
//-------------------------------------------------------

void ExplosionFlashEffect::setColor(int newColor)
{
    color = newColor;

    loadImage();
}

//-----------------------------------------------------

void ExplosionFlashEffect::loadImage()
{
    std::string filename="effects/nova_";
    switch (color)
    {
        case COLOR_BLACK:   filename+="black";break;
        case COLOR_RED:     filename+="red";break;
        case COLOR_GREEN:   filename+="green";break;
        case COLOR_BLUE:    filename+="blue";break;
        case COLOR_PURPLE:  filename+="purple";break;
        case COLOR_YELLOW:  filename+="yellow";break;
        case COLOR_ORANGE:  filename+="orange";break;
        case COLOR_WHITE:   filename+="white";break;
        case COLOR_BOMB:    filename+="brown";break;
        case COLOR_CHROME:
        case COLOR_SHIFTER:
        case COLOR_REFLECTOR:
        case COLOR_PAINTER:  filename+="chrome";break;
        case COLOR_LEVELER:  filename+="red";break;
        case COLOR_BLOCK:    filename+="black";break;

    }
    filename+=".png";

    imageHandle=glbTexManager->insertTexture( filename ,SKIN_UNIVERSAL,true);
}
//-----------------------------------------------------

bool ExplosionFlashEffect::update(float dt, float phase)
{
    if (hasExpired()) return false;

    timeElapsed +=dt;

    setSize();
    angle = 180.*rotationSpeed*phase/PI;
    alpha =255*(duration-timeElapsed)/duration;

    return true;
}
//-----------------------------------------------------
void ExplosionFlashEffect::render(float alphaModifier) const
{
    if ( hasExpired() ) return;

    (glbTexManager->getTexturePtr(imageHandle))->render(
                                                        position.x - rc.w/2,
                                                        position.y - rc.h/2,
                                                        &rc,
                                                        -1,
                                                        alpha*alphaModifier,
                                                        angle);
}

//===================SHINEEFFECT======================================================

FPoint ShineEffect::lastTouch = FPoint(0.,0.);// define and initialize static member


void ShineEffect::setTouchLocation( const FPoint& newTouch)
{
    lastTouch = newTouch;
}
//-------------------------------------------------------------
ShineEffect::ShineEffect (const FPoint& pos,  float maxSize)
{
    dimension=maxSize;
    offset=FPoint(0.,0.);
    sphereMap = std::max(screenRect->w,screenRect->h);


    loadImage();
    setPosition(pos);

    setSize();
    init();
}
//-------------------------------------------------------------
void ShineEffect::init()
{
    timeElapsed=0;
    duration=1.;
    alpha=60;
    luminocityCoeff=1.;
    prevPhase=0.;

}
//-------------------------------------------------------------
void ShineEffect::loadImage()
{
    imageHandle=glbTexManager->insertTexture("effects/glint.png",SKIN_UNIVERSAL,true,true);
}
//-------------------------------------------------------------
void ShineEffect::setSize()
{
    int newSize=dimension - offset.GetLength();
    rc.w=rc.h=1.3*newSize;

    luminocityCoeff = newSize/dimension;

}
//-------------------------------------------------------------
bool ShineEffect::hasExpired() const
{
    return false;
}
//-------------------------------------------------------------
void ShineEffect::setPosition(const FPoint &newPos)
{
    position = newPos;

    FPoint maxScreenDist=FPoint(std::max(screenRect->w-position.x,position.x),
                                std::max(screenRect->h-position.y,position.y)
                                );
    sphereMap =maxScreenDist.GetLength();

    transformEffect(lastTouch);
}
//-------------------------------------------------------------
void ShineEffect::transformEffect(const FPoint &point)
{
    FPoint diff=lastTouch - position;

    offset = diff*(1.*dimension/sphereMap);

    setSize();
}
//-------------------------------------------------------------
void ShineEffect::resize( const SDL_Rect& rc)
{
    transformEffect(lastTouch);
}
//-------------------------------------------------------------
bool ShineEffect::update(float dt, float phase)
{
   return false;//not considered an active effect
}
//-------------------------------------------------------------
void ShineEffect::render(float alphaModifier) const
{
    render(FPoint(0.,0.),alphaModifier);
}
//-------------------------------------------------------------
void ShineEffect::render(const FPoint& posOffset,float alphaModifier) const
{
    (glbTexManager->getTexturePtr(imageHandle))->render(
                                                        position.x+posOffset.x+ offset.x- rc.w/2,
                                                        position.y+posOffset.y+ offset.y- rc.h/2,
                                                        &rc,
                                                        -1,
                                                        alpha*luminocityCoeff*alphaModifier);
}
//-------------------------------------------------------------

//====================================================================================
SDL_Rect SparksEffect::rcWell;  //static member

void SparksEffect::setWellRectangle(const SDL_Rect &well)
{
    rcWell=well;
}
//-------------------------------------------------------------
void SparksEffect::updateSparks(float dt)
{
    generateSparks(dt);

    if (sparkList.empty())
        return;

    PROFILER_START();

    auto spark=sparkList.begin();
    while( spark != sparkList.end() )
    {
        spark->timeAlive+=dt;

        updateSparkAcceleration(*spark);

        if (spark->timeAlive>=dt)
        {
            spark->position += spark->velocity*dt;
            spark->velocity += spark->acceleration*dt;
        }
        else
        {
            spark->position += spark->velocity*spark->timeAlive;
            spark->velocity += spark->acceleration*spark->timeAlive;
        }
        auto curSpark=spark++;// possibly delete the current spark, so advance iterator
        if (
            ( deleteOutOfBounds && outOfBounds(*curSpark) ) ||
            ( myfabs( curSpark->velocity.x*dt) > rcWell.w ) ||
            ( myfabs( curSpark->velocity.y*dt) > rcWell.h ) ||
            ( curSpark->timeAlive >= duration)
           )
          sparkList.erase(curSpark);
    }

    PROFILER_STOP();

}
//-------------------------------------------------------------
bool SparksEffect::outOfBounds (const Spark& spark) const
{
    if ( (spark.position.x <= rcWell.x) ||
         (spark.position.y <= rcWell.y) ||
         (spark.position.x >= rcWell.x + rcWell.w) ||
         (spark.position.y >= rcWell.y + rcWell.h)
        )
        return true;
    return false;
}
//-------------------------------------------------------------
void SparksEffect::deleteExpiredSparks()
{

    if (sparkList.empty())
        return;

    auto spark= sparkList.begin();

    while( spark !=sparkList.end())
    {
        if (spark->timeAlive >= duration )
        {
            auto prev=spark++;
            sparkList.erase(prev);
        }
        else
            spark++;
    }
}
//-------------------------------------------------------------
void SparksEffect::deleteAllSparks()
{
    sparkList.clear();
}
//-------------------------------------------------------------

void SparksEffect::init()
{
    sparkList.clear();
}
//-------------------------------------------------------------
void SparksEffect::reset()
{
    deleteAllSparks();
    timeElapsed=0.;
}
//-------------------------------------------------------------
void SparksEffect::setSize()
{
    rc.w=rc.h=1.3*dimension;
}
//-------------------------------------------------------------
bool SparksEffect::update(float dt, float phase)
{
    if (hasExpired())
        return false;

    updateSparks(dt);
    updateTime(dt);

    return true;
}
//------------------------------------------------------------
void SparksEffect::updateTime(float dt)
{
}
//-------------------------------------------------------------
void SparksEffect::render(float alphaModifier) const
{
    if ( hasExpired() || sparkList.empty() )
        return;

    PROFILER_START();

    float t;

    BaseTexture* tptr =glbTexManager->getTexturePtr(imageHandle);

    int r=rc.w/2;
    int a=alphaModifier*255;

    for (auto spark=sparkList.begin(); spark!=sparkList.end(); ++spark)
    {
        if (outOfBounds(*spark))
            continue;

        t=pow( spark->timeAlive/duration , 3);

        tptr->render(
                      spark->position.x-r,
                      spark->position.y-r,
                      &rc,-1,
                      a*(1.-t)
                      );

    }


    PROFILER_STOP();
}
//===============================================================

FuseEffect::FuseEffect (const FPoint& pos,const FPoint& fusePos,
                           float sparkSize, const FPoint& accel ):fuseOffset(fusePos),timeSinceLastSpark(0.)
{
    position=pos;
    dimension=sparkSize;
    duration=1.;
    acceleration=accel;

    sparkGenDt=((!gameSettings->fullSceneComplexity)?0.06:0.03);

    deleteOutOfBounds=true;
    setSize();
    loadImage();
    init();
    reset();


}
//-------------------------------------------------------------
bool FuseEffect::hasExpired() const
{
    return false;
}
//-------------------------------------------------------------
void FuseEffect::createSpark(float dt)
{

    Spark s;
    s.position=position+fuseOffset;

    float rx=genRand10000();
    float ry=genRand10000();


    s.velocity=FPoint ( (rx-5000.)/100., -ry/100.);
    s.timeAlive = dt;
    s.acceleration=acceleration;
    sparkList.push_back(s);
}
//-------------------------------------------------------------
void FuseEffect::generateSparks(float dt)
{
    if (dt==0.) return;

    timeSinceLastSpark+=dt;

    float creationTime=sparkGenDt-timeSinceLastSpark;

    while (creationTime<=dt)
    {
        createSpark(creationTime-dt);//will be shifted to positive time in updateSparks
        creationTime       +=sparkGenDt;
        timeSinceLastSpark -=sparkGenDt;
    }
}
//-------------------------------------------------------------

void FuseEffect::loadImage()
{
    imageHandle=glbTexManager->insertTexture("effects/spark_yellow.png",SKIN_UNIVERSAL,true);//here
}

//-------------------------------------------------------------
void FuseEffect::setSize()
{
    SparksEffect::setSize();

    rcFuse.w=rcFuse.h=2*dimension;
}

//-------------------------------------------------------------
void FuseEffect::render(float alphaModifier)
{
    SparksEffect::render(alphaModifier);
//render a permanent spark at the fuse point

    glbTexManager->getTexturePtr(imageHandle)->render(
                                                  position.x + fuseOffset.x-rcFuse.w/2.,
                                                  position.y + fuseOffset.y-rcFuse.h/2.,
                                                  &rcFuse,-1,
                                                  alphaModifier*255
                                                  );
}
//========================================================================

ExplosionSparksEffect::ExplosionSparksEffect (const FPoint& pos,int ballColor,
                                              float sparkSize, const FPoint& accel)
{
    duration=1.;
    position=pos;
    acceleration=accel;
    dimension=sparkSize;
    deleteOutOfBounds=true;

    timeElapsed =duration +1.;// start expired

    setSize();
    setColor(ballColor);

    init();
}
//-------------------------------------------------------------
void ExplosionSparksEffect::setColor(int newColor)
{
    color=newColor;

    loadImage();
}
//------------------------------------------------------------
void ExplosionSparksEffect::updateTime(float dt)
{
        timeElapsed+=dt;
}

//-------------------------------------------------------------
void ExplosionSparksEffect::createSpark(float dt)
{
    Spark s;
    s.position=position;
    s.timeAlive = 0.;
    s.acceleration=acceleration;

    float rx=genRand10000();
    float ry=genRand10000();

    float den=5.f + genRand10000()/1000.f;

    if (color==COLOR_LEVELER)
        s.velocity=FPoint ( (rx-5000.)/(0.1*den), (ry-5000.)/(10*den));
    else
        s.velocity=FPoint ( (rx-5000.)/den, (ry-5000.)/den);

    sparkList.push_back(s);

}
//-------------------------------------------------------------
void ExplosionSparksEffect::generateSparks(float dt)
{
    if (dt>=0.) return;

    for (int i=0;i< ((!gameSettings->fullSceneComplexity)?10:20);i++)
    {
        createSpark(0.);//will be shifted to positive time in updateSparks
    }
}
//-------------------------------------------------------------

void ExplosionSparksEffect::loadImage()
{
    std::string filename="effects/spark_";
    switch (color)
    {
        case COLOR_BLACK:   filename+="black";break;
        case COLOR_RED:     filename+="red";break;
        case COLOR_GREEN:   filename+="green";break;
        case COLOR_BLUE:    filename+="blue";break;
        case COLOR_PURPLE:  filename+="purple";break;
        case COLOR_YELLOW:  filename+="yellow";break;
        case COLOR_ORANGE:  filename+="orange";break;
        case COLOR_BOMB:    filename+="brown";break;
        case COLOR_REFLECTOR:
        case COLOR_SHIFTER:
        case COLOR_CHROME:  filename+="chrome";break;
        case COLOR_PAINTER:
        case COLOR_LEVELER:
        case COLOR_WHITE:   filename+="white";break;
        case COLOR_BLOCK:    filename+="black";break;


    }
    filename+=".png";

    imageHandle=glbTexManager->insertTexture( filename ,SKIN_UNIVERSAL,true);
}

void ExplosionSparksEffect::reset()
{
    deleteAllSparks();
    generateSparks(-1.);
    timeElapsed=0.;
}


//=======================================================================

PixiesEffect::PixiesEffect(const FPoint& pos, int sparkColor, float sparkSize)
{
    position=pos;
    dimension=sparkSize;
    acceleration=FPoint(0.,0.);
    duration=2.;
    timeSinceLastSpark=0.;
    color=sparkColor;
    deleteOutOfBounds=true;

    sparkGenDt=((!gameSettings->fullSceneComplexity)?0.12:0.06);

    setSize();
    loadImage();

    init();
    reset();


}
//-------------------------------------------------------------
bool PixiesEffect::hasExpired() const
{
    return false;
}
//-------------------------------------------------------------
void PixiesEffect::createSpark(float dt)
{
    PROFILER_START();

    Spark s;
    s.position=position;

    float rx=genRand10000();
    float ry=genRand10000();

    float modifier;
    FPoint v;

    switch (color)
    {
        case COLOR_CHROME: modifier=6.; v=FPoint( (rx-5000.), (ry-5000.));break;
        case COLOR_SHIFTER:modifier=5.; v=FPoint( (rx-5000.), ry/100. -50);break;//(ry/20-250.));break;
        case COLOR_LEVELER:modifier=8.; v=FPoint( (rx-5000.), (ry/20.-250.));break;
        case COLOR_PAINTER:modifier=3.5; v=FPoint( (rx-5000.), 0.f);break;
    }

    v=v.GetNormal()*modifier*dimension;


    s.position+=v;

    if (color==COLOR_PAINTER)
        modifier=5.;
    else
        modifier=1.;

    s.velocity=FPoint(v.y,v.x)*modifier;

    updateSparkAcceleration(s);


    s.timeAlive = dt;

    sparkList.push_back(s);

    PROFILER_STOP();
}
//-------------------------------------------------------------
void PixiesEffect::generateSparks(float dt)
{
    if (dt==0.) return;

    timeSinceLastSpark+=dt;

    float creationTime=sparkGenDt-timeSinceLastSpark;

    while (creationTime<=dt)
    {
        createSpark(creationTime-dt);//will be shifted to positive time in updateSparks
        creationTime       +=sparkGenDt;
        timeSinceLastSpark -=sparkGenDt;
    }
}
//-------------------------------------------------------------

void PixiesEffect::loadImage()
{
     std::string filename="effects/spark_";
    switch (color)
    {
        case COLOR_BLACK:   filename+="black";break;
        case COLOR_RED:     filename+="red";break;
        case COLOR_GREEN:   filename+="green";break;
        case COLOR_BLUE:    filename+="blue";break;
        case COLOR_PURPLE:  filename+="purple";break;
        case COLOR_YELLOW:  filename+="yellow";break;
        case COLOR_ORANGE:  filename+="orange";break;
        case COLOR_WHITE:   filename+="white";break;
        case COLOR_BOMB:    filename+="brown";break;
        case COLOR_CHROME:  filename+="chrome";break;
        case COLOR_SHIFTER:  filename+="green";break;
        case COLOR_PAINTER:  filename+="white";break;
        case COLOR_LEVELER:  filename+="yellow";break;
        case COLOR_BLOCK:    filename+="black";break;
        case COLOR_REFLECTOR:  filename+="chrome";break;
    }
    filename+=".png";

    imageHandle=glbTexManager->insertTexture( filename ,SKIN_UNIVERSAL,true);//here
}

//-------------------------------------------------------------
void PixiesEffect::setSize()
{
    SparksEffect::setSize();

    rcFuse.w=rcFuse.h=2*dimension;
}

//-------------------------------------------------------------
void PixiesEffect::updateSparkAcceleration (Spark& spark)
{
    FPoint force,dist;
    dist=position - spark.position;
    force = dist.GetNormal()*(dist*dist);//dot product = len squared
    spark.acceleration=force;
}
