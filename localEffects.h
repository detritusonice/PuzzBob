#ifndef LOCAL_EFFECTS_H
#define LOCAL_EFFECTS_H

#include <list>


class LocalEffect
{
protected:
    FPoint position;
    SDL_Rect rc;

    float duration;
    float timeElapsed;
    float dimension;
    int alpha;

    int imageHandle;
    int color;

    virtual void init()=0;
    virtual void loadImage()=0;
    virtual void reset()=0;

public:

    virtual ~LocalEffect(){}

    void start();

    void setAlpha(int newAlpha);
    void deltaAlpha(int diff);

    virtual bool hasExpired() const;

    virtual void setPosition(const FPoint &newPos);
    virtual void transformEffect(const FPoint &point){}

    virtual void setColor(int newColor){};

    virtual void resize( const SDL_Rect& rc)=0;

    virtual void render(float alphaModifier=1.) const =0;
    virtual bool update(float dt=0.f, float phase=0.f)  =0;

};
//========================================================
class ScoreEffect: public LocalEffect
{

    int value;

    void init();
    void loadImage();
    void reset();
    void setSize();

public:
    ScoreEffect (const FPoint& pos,  float maxSize, int value, float duration = 0.3f);
    ~ScoreEffect(){}


    void setPosition(const FPoint &newPos){}

    void resize( const SDL_Rect& rc);

    bool update(float dt, float phase);
    void render(float alphaModifier=1.) const;
};
//========================================================

class ExplosionFlashEffect: public LocalEffect
{
    float angle;
    float rotationSpeed;

    void init();
    void loadImage();
    void reset();

    void setSize();


public:
    ExplosionFlashEffect (const FPoint& pos,  float maxSize, int explosionColor=COLOR_WHITE,
                float duration = 0.3f ,float rotSpeed=0.f );
    ~ExplosionFlashEffect(){}

    void setColor(int newColor);
    void resize( const SDL_Rect& rc);

    bool update(float dt, float phase);
    void render(float alphaModifier=1.) const;

};

//========================================================

class ShineEffect : public LocalEffect
{

    FPoint offset;
    float sphereMap;
    float prevPhase;
    float luminocityCoeff;

    void init();
    void loadImage();
    void reset(){}

    void setSize();


public:
    static FPoint lastTouch;


    ShineEffect (const FPoint& pos,  float maxSize);
    ~ShineEffect(){}

    static void setTouchLocation(const FPoint& newTouch);
    bool hasExpired() const;

    void setPosition(const FPoint &newPos);
    void transformEffect(const FPoint &point);

    void setColor(int newColor){}
    void resize( const SDL_Rect& rc);

    bool update(float dt, float phase);
    void render(const FPoint& posOffset,float alphaModifier=1.) const;
    void render(float alphaModifier=1.) const;

};

//========================================================
struct Spark
{
    FPoint position;
    FPoint velocity;
    FPoint acceleration;
    float timeAlive;
};

class SparksEffect : public LocalEffect
{
protected:

    std::list<Spark> sparkList;

    static SDL_Rect rcWell;

    FPoint acceleration;
    bool deleteOutOfBounds;

    void init();
    void reset();

    void setSize();
    virtual void createSpark(float dt)=0;
    virtual void generateSparks(float dt)=0;
    void updateSparks(float dt);
    bool outOfBounds(const Spark& spark) const;

    void deleteExpiredSparks();
    void deleteAllSparks();

    virtual void updateTime(float dt);
    virtual void updateSparkAcceleration(Spark& spark) {};
public:

    ~SparksEffect(){}// reduced to this after using std::list

    static void setWellRectangle(const SDL_Rect &well);

    void resize( const SDL_Rect& rc){}
    void setColor(int newColor){}

    void transformEffect(const FPoint &point){}

    bool update(float dt, float phase);
    void render(float alphaModifier=1.) const;


};
//--------------------------------------------------------------
class FuseEffect : public SparksEffect
{

    SDL_Rect rcFuse;
    float sparkGenDt;
    float timeSinceLastSpark;

    FPoint fuseOffset;

    void createSpark(float dt);
    void generateSparks(float dt);


    void loadImage();
    void setSize();


public:
    FuseEffect(const FPoint& pos,const FPoint& fusePos,float sparkSize, const FPoint& accel );

    FuseEffect( const FuseEffect&)            =delete;
    FuseEffect& operator =(const FuseEffect&) = delete;


    bool hasExpired() const;

    void render(float alphaModifier=1.);
};

//--------------------------------------------------------------
class PixiesEffect : public SparksEffect
{

    SDL_Rect rcFuse;
    float sparkGenDt;
    float timeSinceLastSpark;

    void createSpark(float dt);
    void generateSparks(float dt);


    void loadImage();
    void setSize();

    void updateSparkAcceleration (Spark& spark);
    void dealWithOutOfBounds( Spark& s){};

public:
    PixiesEffect(const FPoint& pos, int sparkColor, float sparkSize);

    PixiesEffect( const PixiesEffect&)            =delete;
    PixiesEffect& operator =(const PixiesEffect&) = delete;


    bool hasExpired() const;
};
//-----------------------------------------------------------------
class ExplosionSparksEffect : public SparksEffect
{
    void createSpark(float dt);
    void generateSparks(float dt);

    void loadImage();
    void updateTime(float dt);
    void reset();
public:
    ExplosionSparksEffect(const FPoint& pos,int ballColor,float sparkSize, const FPoint& accel);

    ExplosionSparksEffect( const ExplosionSparksEffect&)            =delete;
    ExplosionSparksEffect& operator =(const ExplosionSparksEffect&) = delete;


    void setColor(int newColor);
};
#endif // LOCAL_EFFECTS_H
