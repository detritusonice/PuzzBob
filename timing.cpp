#include "base.h"
#include "timing.h"

//---------------------------------------

Timer::Timer()
{
    startTicks  = 0;
    pausedTicks = 0;

    started = false;
    paused  = false;

}

//---------------------------------------

void Timer::Start()
{
    started = true;
    paused  = false;

    startTicks  = SDL_GetTicks();
}

//---------------------------------------

void Timer::Stop()
{
    started = false;
    paused  = false;

    startTicks  = 0;
}

//---------------------------------------

void Timer::Pause()
{
    if ( ( started == true ) && ( paused == false ) )
    {
        paused  = true;
        pausedTicks = SDL_GetTicks()-startTicks;
    }
}

//---------------------------------------

void Timer::Resume()
{
    if ( ( started == true ) && ( paused == true ) )
    {
        startTicks  = SDL_GetTicks()-pausedTicks;
        pausedTicks = 0;
        paused  = false;
    }
}

//---------------------------------------

int Timer::GetTicks()
{
    if ( started == true )
    {
        if ( paused == true )
            return pausedTicks;
        else
            return SDL_GetTicks()-startTicks;
    }
    return 0;
}

//---------------------------------------

bool Timer::IsStarted()
{
    return started;
}

//---------------------------------------

bool Timer::IsPaused()
{
    return paused;
}

//---------------------------------------
void Timer::SetPausedTicks(int ticks)
{
    started=true;
    paused=true;
    pausedTicks=ticks;
}
