#include "base.h"
#include "basefuncs.h"
#include "StateMachine.h"
#include "antihack.h"



SDL_Window *glbWindow = NULL;

SDL_Renderer *glbRenderer =NULL;

TTF_Font *glbFont = NULL;
TTF_Font *glbMessageFont = NULL;

SDL_Cursor* glbGameCursor = NULL;



int main(int argc, char *argv[])
{
#ifdef PLATFORM_ANDROID
    ahGetParams();
#endif // PLATFORM_ANDROID
    if(init()) //couple of atexit calls in here
    {
        if( loadFiles())
        {
            StateMachine theMachine;
            theMachine.Run();
        }
    }

	cleanUp();

    return 0;
}

