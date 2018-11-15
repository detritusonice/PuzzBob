#include "base.h"
#include "states.h"

#include "stateMachine.h"

//=========================== STATE CLASS IMPLEMENTATIONS ======================================

//============== ENTRY IMPLEMENTATION ============

Entry::Entry()
{
    isHibernating=false;
}

//-----------------------------------------------

Entry::~Entry()
{
}

//-----------------------------------------------

void Entry::HandleEvents()
{
    StateMachine::SetNextState(STATE_TITLE);
}

//-----------------------------------------------

void Entry::Logic(float dt,float phase)
{
}

//-----------------------------------------------

void Entry::Render(float phase)
{
    SDL_SetRenderDrawColor(glbRenderer,0,0,0,0xFF);
    SDL_RenderClear(glbRenderer);
#ifndef PROFILING
    SDL_RenderPresent(glbRenderer);
#endif
}


//============== END OF ENTRY IMPLEMENTATION ============
