#ifndef STATES_H
#define STATES_H

//================ BASIC STATE CLASS DEFINITIONS =============================
//======== FUNCTIONAL STATES HAVE THEIR OWN FILE PAIRS========================
extern bool isHibernating;

class GameState
{
    protected:

    public:
    virtual void HandleEvents() = 0;
    virtual void Logic(float dt , float phase=0.) = 0;
    virtual void Render(float phase=0.) = 0;
    virtual ~GameState(){};
};

//-----------------------------------------------

class Entry: public GameState
{

    public:

    Entry();
    ~Entry();
    void Logic(float dt, float phase=0.);
    void Render(float phase=0.);
    void HandleEvents();
};

#endif
