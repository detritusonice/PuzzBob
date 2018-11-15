#ifndef BASE_H
#define BASE_H


#include <SDL.h>            //basic sdl
#include <SDL_image.h>      //image extension
#include <SDL_ttf.h>        //TrueTypeFont extension
#include <SDL_mixer.h>      //sound extension

// EXTRA_GAME_MODES is defined in constants.h

#include "platform.h"
#include "constants.h"      //game constants (screen, gameplay etc)
#include "globals.h"


#ifndef HACK_PROTECTION
    #define HACK_PROTECTION
#endif

#ifndef PLATFORM_PC
    #ifndef PLATFORM_ANDROID
        #define PLATFORM_ANDROID
    #endif // PLATFORM_ANDROID
#endif


#ifndef CAP_FRAMERATE
    #define CAP_FRAMERATE
#endif

#ifndef USE_WATERMARK
    #define USE_WATERMARK
#endif

#ifndef FILE_LOG
  #define FILE_LOG
#endif

#ifndef CONSOLE_LOG
   //#define CONSOLE_LOG//HERE
#endif

#ifndef HACK_PROTECTION
    #ifndef PROFILING
       #define PROFILING
    #endif
#endif

#ifdef PROFILING
    #include <typeinfo>
    #include "profiler.h"

    #define PROFILER_START() glbProfiler->startRec( typeid(*this).name() ,__func__)
    #define PROFILER_STOP()  glbProfiler->stopRec( typeid(*this).name() ,__func__)
    #define PROFILER_ARB_START( M ) glbProfiler->startRec( "" , M )
    #define PROFILER_ARB_STOP( M ) glbProfiler->stopRec( "" , M )
#else
    #define PROFILER_START() ;
    #define PROFILER_STOP() ;
    #define PROFILER_ARB_START(M) ;
    #define PROFILER_ARB_STOP(M) ;
#endif


#ifndef DEMO_GAME
     //#define DEMO_GAME
#endif


#define MAX_DEMO_LEVEL 18


#ifndef DEMO_GAME
    #ifndef RANDOM_LEVELS
        #define RANDOM_LEVELS
    #endif
#endif

#ifndef HACK_PROTECTION
    #ifndef DEBUGMODE
       // #define DEBUGMODE//HERE
    #endif
#endif

#ifdef DEBUGMODE
    #include <typeinfo>
    #ifdef CONSOLE_LOG
        #define DEBUG_REC() { sprintf(glbLogMsg,"%s\n",__func__); mySDL_Log(glbLogMsg);}
    #else
        #define DEBUG_REC() SDL_Log("%s\n",__func__);
    #endif
#else
    #define DEBUG_REC() ;
#endif

#endif
